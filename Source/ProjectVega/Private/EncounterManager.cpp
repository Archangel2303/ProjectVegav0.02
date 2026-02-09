#include "EncounterManager.h"
#include "ProjectVegaRunStateSubsystem.h"
#include "EncounterDefinitionDataAsset.h"
#include "FloorMapTypes.h"
#include "ProjectVegaPlayerCharacter.h"
#include "AITurnComponent.h"
#include "TurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "AttributeComponent.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "ProjectVegaDeathWidget.h"
#include "ProjectVegaRewardWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"

AEncounterManager::AEncounterManager()
{
    PrimaryActorTick.bCanEverTick = false;

    PlayerSpawnRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerSpawnRoot"));
    EnemySpawnRoot = CreateDefaultSubobject<USceneComponent>(TEXT("EnemySpawnRoot"));
    RootComponent = PlayerSpawnRoot;
    EnemySpawnRoot->SetupAttachment(RootComponent);
}

void AEncounterManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartOnBeginPlay)
    {
        StartEncounterFromRunState();
    }
}

void AEncounterManager::StartEncounterFromRunState()
{
    UProjectVegaRunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>() : nullptr;
    if (!RunState)
    {
        if (bLogEncounter)
        {
            UE_LOG(LogTemp, Warning, TEXT("EncounterManager: RunState subsystem missing"));
        }
        return;
    }

    EnsurePlayerSpawned();
    if (PlayerCharacter)
    {
        RunState->ApplyLoadout(PlayerCharacter);
    }

    UEncounterDefinitionDataAsset* Encounter = RunState->GetSelectedEncounter();
    if (Encounter)
    {
        SpawnEnemies(Encounter);
    }
    else if (bLogEncounter)
    {
        UE_LOG(LogTemp, Warning, TEXT("EncounterManager: No selected encounter"));
    }

    BindTurnManager();

    bEncounterEnded = false;
}

void AEncounterManager::ResolveEncounterDebug(bool bSkipRewards)
{
    if (bEncounterEnded)
    {
        return;
    }

    bEncounterEnded = true;
    if (bSkipRewards)
    {
        if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
        {
            RunState->MarkEncounterResolved(true);
        }
        return;
    }

    ShowRewardPrompt();
}

void AEncounterManager::EnsurePlayerSpawned()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        if (bLogEncounter)
        {
            UE_LOG(LogTemp, Warning, TEXT("EncounterManager: PlayerController missing"));
        }
        return;
    }

    PlayerCharacter = Cast<AProjectVegaPlayerCharacter>(PC->GetPawn());
    TArray<FTransform> PlayerSpawns;
    CollectSpawnTransforms(PlayerSpawnRoot, PlayerSpawns);

    if (!PlayerCharacter)
    {
        if (PlayerSpawns.Num() > 0)
        {
            const FTransform& SpawnTransform = PlayerSpawns[0];
            PlayerCharacter = GetWorld()->SpawnActor<AProjectVegaPlayerCharacter>(AProjectVegaPlayerCharacter::StaticClass(), SpawnTransform);
            if (PlayerCharacter)
            {
                PC->Possess(PlayerCharacter);
            }
        }
        else if (bLogEncounter)
        {
            UE_LOG(LogTemp, Warning, TEXT("EncounterManager: No player spawn points found"));
        }
    }

    if (PlayerCharacter && PlayerSpawns.Num() > 0)
    {
        const FTransform& SpawnTransform = PlayerSpawns[0];
        PlayerCharacter->SetActorLocationAndRotation(SpawnTransform.GetLocation(), SpawnTransform.GetRotation());
    }
}

void AEncounterManager::SpawnEnemies(UEncounterDefinitionDataAsset* Encounter)
{
    if (!Encounter)
    {
        if (bLogEncounter)
        {
            UE_LOG(LogTemp, Warning, TEXT("EncounterManager: Encounter asset missing"));
        }
        return;
    }

    TArray<FTransform> EnemySpawns;
    CollectSpawnTransforms(EnemySpawnRoot, EnemySpawns);
    if (EnemySpawns.Num() == 0)
    {
        if (bLogEncounter)
        {
            UE_LOG(LogTemp, Warning, TEXT("EncounterManager: No enemy spawn points found"));
        }
        return;
    }

    SpawnedEnemies.Reset();
    int32 SpawnIndex = 0;

    for (const FEnemySpawnEntry& Entry : Encounter->Enemies)
    {
        if (!Entry.EnemyClass)
        {
            if (bLogEncounter)
            {
                UE_LOG(LogTemp, Warning, TEXT("EncounterManager: Encounter '%s' has null EnemyClass"), *Encounter->EncounterName.ToString());
            }
            continue;
        }

        const int32 Count = FMath::Clamp(FMath::RandRange(Entry.MinCount, Entry.MaxCount), 1, 50);
        for (int32 i = 0; i < Count; ++i)
        {
            const FTransform& SpawnTransform = EnemySpawns[SpawnIndex % EnemySpawns.Num()];
            AActor* Spawned = GetWorld()->SpawnActor<AActor>(Entry.EnemyClass, SpawnTransform);
            if (Spawned)
            {
                SpawnedEnemies.Add(Spawned);
            }
            else if (bLogEncounter)
            {
                UE_LOG(LogTemp, Warning, TEXT("EncounterManager: Failed to spawn %s"), *Entry.EnemyClass->GetName());
            }
            ++SpawnIndex;
        }
    }

    if (bLogEncounter)
    {
        UE_LOG(LogTemp, Log, TEXT("EncounterManager: Spawned %d enemies for %s"), SpawnedEnemies.Num(), *Encounter->EncounterName.ToString());
    }
}

void AEncounterManager::BindTurnManager()
{
    if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
        {
            TurnManager->OnTurnAdvanced.AddDynamic(this, &AEncounterManager::HandleTurnAdvanced);
        }
    }
}

void AEncounterManager::HandleTurnAdvanced(int32 TurnNumber)
{
    if (CheckEncounterEndState())
    {
        return;
    }

    bPlayerTurn = !bPlayerTurn;
    if (!bPlayerTurn)
    {
        ExecuteAITurn();
    }
}

void AEncounterManager::ExecuteAITurn()
{
    if (SpawnedEnemies.Num() == 0 || !PlayerCharacter)
    {
        return;
    }

    AActor* Target = SelectAITarget();
    if (!Target)
    {
        Target = PlayerCharacter;
    }

    const int32 StartIndex = NextEnemyIndex;
    for (int32 i = 0; i < SpawnedEnemies.Num(); ++i)
    {
        const int32 Index = (StartIndex + i) % SpawnedEnemies.Num();
        AActor* Candidate = SpawnedEnemies[Index];
        if (!Candidate)
        {
            continue;
        }

        if (UAITurnComponent* AITurn = Candidate->FindComponentByClass<UAITurnComponent>())
        {
            NextEnemyIndex = (Index + 1) % SpawnedEnemies.Num();
            AITurn->PerformAITurn(Target);
            return;
        }
    }
}

bool AEncounterManager::IsActorDefeated(AActor* Actor) const
{
    if (!Actor)
    {
        return true;
    }

    if (UAttributeComponent* Attr = Actor->FindComponentByClass<UAttributeComponent>())
    {
        return Attr->GetAttribute(TEXT("Health")) <= 0.f;
    }

    return Actor->IsActorBeingDestroyed();
}

bool AEncounterManager::CheckEncounterEndState()
{
    if (bEncounterEnded)
    {
        return true;
    }

    if (IsActorDefeated(PlayerCharacter))
    {
        bEncounterEnded = true;
        ShowDeathPrompt();
        return true;
    }

    for (int32 i = SpawnedEnemies.Num() - 1; i >= 0; --i)
    {
        if (IsActorDefeated(SpawnedEnemies[i]))
        {
            SpawnedEnemies.RemoveAt(i);
        }
    }

    if (SpawnedEnemies.Num() == 0)
    {
        bEncounterEnded = true;
        ShowRewardPrompt();
        return true;
    }

    return false;
}

void AEncounterManager::ShowDeathPrompt()
{
    if (!DeathWidgetClass)
    {
        DeathWidgetClass = UProjectVegaDeathWidget::StaticClass();
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (UProjectVegaDeathWidget* Widget = CreateWidget<UProjectVegaDeathWidget>(PC, DeathWidgetClass))
        {
            Widget->OnRetry.AddDynamic(this, &AEncounterManager::HandleDeathRetry);
            Widget->OnQuit.AddDynamic(this, &AEncounterManager::HandleDeathQuit);
            Widget->AddToViewport(1000);
        }
    }
}

void AEncounterManager::ShowRewardPrompt()
{
    if (!RewardWidgetClass)
    {
        RewardWidgetClass = UProjectVegaRewardWidget::StaticClass();
    }

    if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
    {
        const int32 Nanites = RunState->RollRewardNanites();
        const TArray<UAugmentDataAsset*> Augments = RunState->RollRewardAugments(3);

        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            if (UProjectVegaRewardWidget* Widget = CreateWidget<UProjectVegaRewardWidget>(PC, RewardWidgetClass))
            {
                Widget->InitializeReward(Nanites, Augments);
                Widget->OnNanitesAccepted.AddDynamic(this, &AEncounterManager::HandleRewardNanites);
                Widget->OnAugmentChosen.AddDynamic(this, &AEncounterManager::HandleRewardAugment);
                Widget->OnContinue.AddDynamic(this, &AEncounterManager::HandleRewardContinue);
                Widget->OnVialOpened.AddDynamic(this, &AEncounterManager::HandleRewardVialOpened);
                Widget->AddToViewport(900);
                ActiveRewardWidget = Widget;

                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(Widget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
    }
}

void AEncounterManager::HandleDeathRetry()
{
    if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
    {
        RunState->ResetRun(true);
        RunState->ReturnToFloorMap(true);
    }
}

void AEncounterManager::HandleDeathQuit()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}

void AEncounterManager::HandleRewardNanites(int32 Amount)
{
    if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
    {
        RunState->AddNanites(Amount);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("Picked up %d nanites"), Amount));
        }
    }
}

void AEncounterManager::HandleRewardAugment(UAugmentDataAsset* Augment)
{
    if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
    {
        RunState->AddAugmentToInventory(Augment);
        if (GEngine)
        {
            const FString Name = Augment ? Augment->GetName() : TEXT("(None)");
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
                FString::Printf(TEXT("Picked up augment: %s"), *Name));
        }
    }
}

void AEncounterManager::HandleRewardVialOpened()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("Opened nanite vial"));
    }
}

void AEncounterManager::HandleRewardContinue()
{
    if (ActiveRewardWidget)
    {
        ActiveRewardWidget->RemoveFromParent();
        ActiveRewardWidget = nullptr;
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    if (UProjectVegaRunStateSubsystem* RunState = GetWorld()->GetGameInstance()->GetSubsystem<UProjectVegaRunStateSubsystem>())
    {
        RunState->MarkEncounterResolved(true);
    }
}

void AEncounterManager::CollectSpawnTransforms(const USceneComponent* Root, TArray<FTransform>& OutTransforms) const
{
    OutTransforms.Reset();

    if (!Root)
    {
        return;
    }

    TArray<USceneComponent*> ChildComponents;
    Root->GetChildrenComponents(false, ChildComponents);
    if (ChildComponents.Num() == 0)
    {
        OutTransforms.Add(Root->GetComponentTransform());
        return;
    }

    for (USceneComponent* Child : ChildComponents)
    {
        if (Child)
        {
            OutTransforms.Add(Child->GetComponentTransform());
        }
    }
}

AActor* AEncounterManager::SelectAITarget() const
{
    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectVegaPlayerCharacter::StaticClass(), Candidates);

    if (Candidates.Num() == 0 && PlayerCharacter)
    {
        Candidates.Add(PlayerCharacter);
    }

    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    float BestScore = -1.f;
    AActor* BestTarget = nullptr;

    for (AActor* Candidate : Candidates)
    {
        const float Score = ScoreTarget(Candidate);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }

    if (bLogAITargeting && BestTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("EncounterManager: AI target %s (score %.2f)"), *BestTarget->GetName(), BestScore);
    }

    return BestTarget;
}

float AEncounterManager::ScoreTarget(AActor* Target) const
{
    if (!Target)
    {
        return 0.f;
    }

    float ThreatScore = 1.f;
    if (UAttributeComponent* Attr = Target->FindComponentByClass<UAttributeComponent>())
    {
        const float Threat = Attr->GetAttribute(TEXT("Threat"));
        ThreatScore = FMath::Max(1.f, Threat);

        const float Health = Attr->GetAttribute(TEXT("Health"));
        const float MaxHealth = FMath::Max(1.f, Attr->GetAttribute(TEXT("MaxHealth")));
        const float MissingPct = 1.f - FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
        ThreatScore += MissingPct * 0.5f;
    }

    float RoleBonus = 0.f;
    if (Target->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
    {
        FGameplayTagContainer OwnedTags;
        if (const IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Target))
        {
            TagIface->GetOwnedGameplayTags(OwnedTags);
        }

        const FGameplayTag TankTag = FGameplayTag::RequestGameplayTag(TEXT("Role.Tank"), false);
        const FGameplayTag HealerTag = FGameplayTag::RequestGameplayTag(TEXT("Role.Healer"), false);
        const FGameplayTag DpsTag = FGameplayTag::RequestGameplayTag(TEXT("Role.DPS"), false);

        if (TankTag.IsValid() && OwnedTags.HasTag(TankTag))
        {
            RoleBonus += 2.5f;
        }
        if (HealerTag.IsValid() && OwnedTags.HasTag(HealerTag))
        {
            RoleBonus += 1.5f;
        }
        if (DpsTag.IsValid() && OwnedTags.HasTag(DpsTag))
        {
            RoleBonus += 1.f;
        }
    }

    return ThreatScore + RoleBonus;
}
