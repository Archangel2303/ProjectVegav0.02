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

    BindTurnManager();
}

void AEncounterManager::EnsurePlayerSpawned()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
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
        return;
    }

    TArray<FTransform> EnemySpawns;
    CollectSpawnTransforms(EnemySpawnRoot, EnemySpawns);
    if (EnemySpawns.Num() == 0)
    {
        return;
    }

    SpawnedEnemies.Reset();
    int32 SpawnIndex = 0;

    for (const FEnemySpawnEntry& Entry : Encounter->Enemies)
    {
        if (!Entry.EnemyClass)
        {
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
            ++SpawnIndex;
        }
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
