#include "ProjectVegaPlayerController.h"
#include "TurnEndWidget.h"
#include "ProjectVegaBattleMenuWidget.h"
#include "ProjectVegaFloorMapWidget.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaPlayerCharacter.h"
#include "AbilityExecutorComponent.h"
#include "AbilityDataAsset.h"
#include "ProjectVegaCombatOverlayWidget.h"
#include "ProjectVegaRewardWidget.h"
#include "EncounterManager.h"
#include "TurnManager.h"
#include "ProjectVegaRunStateSubsystem.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

AProjectVegaPlayerController::AProjectVegaPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    PrimaryActorTick.bCanEverTick = true;
    bShowFloorMapWidget = false;
}

void AProjectVegaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("BattleMenu: BeginPlay for %s"), *GetName());

    UProjectVegaRunStateSubsystem* RunState = nullptr;
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>();
    }

    const FName CurrentLevelName(*UGameplayStatics::GetCurrentLevelName(this, true));

    const bool bIsEncounterLevel = IsEncounterLevel(RunState);
    const bool bIsFloorMapLevel = !bIsEncounterLevel && IsFloorMapLevel(RunState);

    auto RemoveWidgetIfValid = [](auto*& Widget)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
            Widget = nullptr;
        }
    };

    if (bIsFloorMapLevel)
    {
        if (RunState)
        {
            RunState->CacheFloorMapName(CurrentLevelName);
        }
        RemoveWidgetIfValid(BattleMenuWidget);
        RemoveWidgetIfValid(EndTurnWidget);
        RemoveWidgetIfValid(CombatOverlayWidget);
        bCombatMapVisible = false;
        if (FloorMapWidget)
        {
            FloorMapWidget->SetUseBackdrop(false);
        }
    }
    else
    {
        if (RunState && bIsEncounterLevel)
        {
            RunState->CacheEncounterMapName(CurrentLevelName);
        }
        if (FloorMapWidget)
        {
            FloorMapWidget->SetVisibility(ESlateVisibility::Collapsed);
            bCombatMapVisible = false;
        }
    }

    if (EndTurnWidgetClass && !bIsFloorMapLevel)
    {
        EndTurnWidget = CreateWidget<UTurnEndWidget>(this, EndTurnWidgetClass);
        if (EndTurnWidget)
        {
            EndTurnWidget->AddToViewport();
        }
    }

    if (!bIsFloorMapLevel)
    {
        if (!BattleMenuWidgetClass)
        {
            BattleMenuWidgetClass = UProjectVegaBattleMenuWidget::StaticClass();
        }

        if (BattleMenuWidgetClass)
        {
            BattleMenuWidget = CreateWidget<UProjectVegaBattleMenuWidget>(this, BattleMenuWidgetClass);
            if (BattleMenuWidget)
            {
                UE_LOG(LogTemp, Log, TEXT("BattleMenu: widget created"));
                BattleMenuWidget->OnAbilitySelected.AddDynamic(this, &AProjectVegaPlayerController::HandleAbilitySelected);
                BattleMenuWidget->OnEndTurnRequested.AddDynamic(this, &AProjectVegaPlayerController::HandleEndTurnRequested);
                BattleMenuWidget->AddToViewport(200);
                RefreshBattleMenuAbilities();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BattleMenu: widget NOT created"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BattleMenu: widget class missing"));
        }
    }

    if (bIsFloorMapLevel)
    {
        if (!FloorMapWidgetClass)
        {
            FloorMapWidgetClass = UProjectVegaFloorMapWidget::StaticClass();
        }

        if (FloorMapWidgetClass)
        {
            FloorMapWidget = CreateWidget<UProjectVegaFloorMapWidget>(this, FloorMapWidgetClass);
            if (FloorMapWidget)
            {
                FloorMapWidget->SetUseBackdrop(false);
                FloorMapWidget->AddToViewport(10);
                FloorMapWidget->SetVisibility(ESlateVisibility::Visible);
                FloorMapWidget->SetRunState(RunState);
            }
        }
    }
    else
    {
        if (FloorMapWidget)
        {
            FloorMapWidget->SetVisibility(ESlateVisibility::Collapsed);
            bCombatMapVisible = false;
        }

        if (!CombatOverlayWidgetClass)
        {
            CombatOverlayWidgetClass = UProjectVegaCombatOverlayWidget::StaticClass();
        }

        if (CombatOverlayWidgetClass)
        {
            CombatOverlayWidget = CreateWidget<UProjectVegaCombatOverlayWidget>(this, CombatOverlayWidgetClass);
            if (CombatOverlayWidget)
            {
                CombatOverlayWidget->OnToggleMap.AddDynamic(this, &AProjectVegaPlayerController::HandleToggleCombatMap);
                CombatOverlayWidget->OnResolveEncounter.AddDynamic(this, &AProjectVegaPlayerController::HandleResolveEncounterDebug);
                CombatOverlayWidget->AddToViewport(300);
            }
        }
    }
}

void AProjectVegaPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!IsLocalController())
    {
        return;
    }

    UpdateTargetingPreview();
}

void AProjectVegaPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent)
    {
        return;
    }

    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AProjectVegaPlayerController::SelectAbilityIndex1);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AProjectVegaPlayerController::SelectAbilityIndex2);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AProjectVegaPlayerController::SelectAbilityIndex3);
    InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AProjectVegaPlayerController::SelectAbilityIndex4);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AProjectVegaPlayerController::HandleTargetClick);
}

void AProjectVegaPlayerController::SelectAbilityIndex1()
{
    SelectAbilityByIndex(0);
}

void AProjectVegaPlayerController::SelectAbilityIndex2()
{
    SelectAbilityByIndex(1);
}

void AProjectVegaPlayerController::SelectAbilityIndex3()
{
    SelectAbilityByIndex(2);
}

void AProjectVegaPlayerController::SelectAbilityIndex4()
{
    SelectAbilityByIndex(3);
}

void AProjectVegaPlayerController::SelectAbilityByIndex(int32 Index)
{
    AProjectVegaPlayerCharacter* PC = Cast<AProjectVegaPlayerCharacter>(GetPawn());
    if (!PC)
    {
        return;
    }

    TArray<UAbilityDataAsset*> Abilities = PC->GetAvailableAbilities();
    if (!Abilities.IsValidIndex(Index))
    {
        return;
    }

    SelectedAbility = Abilities[Index];
}

void AProjectVegaPlayerController::HandleTargetClick()
{
    if (!SelectedAbility)
    {
        return;
    }

    AProjectVegaPlayerCharacter* PC = Cast<AProjectVegaPlayerCharacter>(GetPawn());
    if (!PC)
    {
        return;
    }

    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        return;
    }

    AActor* Target = Hit.GetActor();
    if (!Target)
    {
        return;
    }

    if (UAbilityExecutorComponent* Exec = PC->GetAbilityExecutor())
    {
        TArray<AActor*> Targets;
        Targets.Add(Target);
        Exec->ExecuteAbilityByTargets(SelectedAbility, Targets, 1.f);
    }
}

void AProjectVegaPlayerController::HandleAbilitySelected(UAbilityDataAsset* Ability)
{
    SelectedAbility = Ability;
}

void AProjectVegaPlayerController::HandleEndTurnRequested()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->EndTurn();
            }
        }
    }

    RefreshBattleMenuAbilities();
}

void AProjectVegaPlayerController::HandleToggleCombatMap()
{
    if (!FloorMapWidget)
    {
        if (!FloorMapWidgetClass)
        {
            FloorMapWidgetClass = UProjectVegaFloorMapWidget::StaticClass();
        }

        if (FloorMapWidgetClass)
        {
            FloorMapWidget = CreateWidget<UProjectVegaFloorMapWidget>(this, FloorMapWidgetClass);
            if (FloorMapWidget)
            {
                if (UGameInstance* GI = GetWorld()->GetGameInstance())
                {
                    if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
                    {
                        FloorMapWidget->SetRunState(RunState);
                    }
                }
                FloorMapWidget->SetUseBackdrop(true);
                FloorMapWidget->AddToViewport(5);
            }
        }
    }

    if (FloorMapWidget)
    {
        bCombatMapVisible = !bCombatMapVisible;
        FloorMapWidget->SetVisibility(bCombatMapVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void AProjectVegaPlayerController::HandleResolveEncounterDebug()
{
    if (AEncounterManager* Manager = Cast<AEncounterManager>(UGameplayStatics::GetActorOfClass(this, AEncounterManager::StaticClass())))
    {
        Manager->ResolveEncounterDebug(false);
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("Resolve: EncounterManager not found; using RunState fallback."));
    }

    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
        {
            const int32 Nanites = RunState->RollRewardNanites();
            const TArray<UAugmentDataAsset*> Augments = RunState->RollRewardAugments(3);

            if (UProjectVegaRewardWidget* Widget = CreateWidget<UProjectVegaRewardWidget>(this, UProjectVegaRewardWidget::StaticClass()))
            {
                Widget->InitializeReward(Nanites, Augments);
                Widget->OnNanitesAccepted.AddDynamic(this, &AProjectVegaPlayerController::HandleRewardNanites);
                Widget->OnAugmentChosen.AddDynamic(this, &AProjectVegaPlayerController::HandleRewardAugment);
                Widget->OnContinue.AddDynamic(this, &AProjectVegaPlayerController::HandleRewardContinue);
                Widget->OnVialOpened.AddDynamic(this, &AProjectVegaPlayerController::HandleRewardVialOpened);
                Widget->AddToViewport(900);
                ActiveRewardWidget = Widget;

                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(Widget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
                bShowMouseCursor = true;
                return;
            }

            RunState->MarkEncounterResolved(true);
        }
    }
}

void AProjectVegaPlayerController::HandleRewardNanites(int32 Amount)
{
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
        {
            RunState->AddNanites(Amount);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                    FString::Printf(TEXT("Picked up %d nanites"), Amount));
            }
        }
    }
}

void AProjectVegaPlayerController::HandleRewardAugment(UAugmentDataAsset* Augment)
{
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
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
}

void AProjectVegaPlayerController::HandleRewardVialOpened()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("Opened nanite vial"));
    }
}

void AProjectVegaPlayerController::HandleRewardContinue()
{
    if (ActiveRewardWidget)
    {
        ActiveRewardWidget->RemoveFromParent();
        ActiveRewardWidget = nullptr;
    }

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
        {
            RunState->MarkEncounterResolved(true);
        }
    }
}

bool AProjectVegaPlayerController::IsFloorMapLevel(const UProjectVegaRunStateSubsystem* RunState) const
{
    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (LevelName.Contains(TEXT("Encounter"), ESearchCase::IgnoreCase))
    {
        return false;
    }

    if (!RunState)
    {
        if (LevelName.Contains(TEXT("Floormap"), ESearchCase::IgnoreCase))
        {
            return true;
        }
        return bShowFloorMapWidget;
    }
    const FName FloorMapName = RunState->GetFloorMapName();
    const FName EncounterMapName = RunState->GetEncounterMapName();

    if (!EncounterMapName.IsNone())
    {
        const FString EncounterPath = EncounterMapName.ToString();
        const FString EncounterShort = FPackageName::GetShortName(EncounterPath);
        if (LevelName.Equals(EncounterPath, ESearchCase::IgnoreCase)
            || LevelName.Equals(EncounterShort, ESearchCase::IgnoreCase))
        {
            return false;
        }
    }

    if (FloorMapName.IsNone())
    {
        if (LevelName.Contains(TEXT("Floormap"), ESearchCase::IgnoreCase))
        {
            return true;
        }
        return bShowFloorMapWidget;
    }

    const FString FloorMapPath = FloorMapName.ToString();
    const FString FloorMapShort = FPackageName::GetShortName(FloorMapPath);
    return LevelName.Equals(FloorMapPath, ESearchCase::IgnoreCase)
        || LevelName.Equals(FloorMapShort, ESearchCase::IgnoreCase);
}

bool AProjectVegaPlayerController::IsEncounterLevel(const UProjectVegaRunStateSubsystem* RunState) const
{
    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (LevelName.Contains(TEXT("Encounter"), ESearchCase::IgnoreCase))
    {
        return true;
    }

    if (!RunState)
    {
        return false;
    }

    const FName EncounterMapName = RunState->GetEncounterMapName();
    if (EncounterMapName.IsNone())
    {
        return false;
    }
    const FString EncounterPath = EncounterMapName.ToString();
    const FString EncounterShort = FPackageName::GetShortName(EncounterPath);
    return LevelName.Equals(EncounterPath, ESearchCase::IgnoreCase)
        || LevelName.Equals(EncounterShort, ESearchCase::IgnoreCase);
}

void AProjectVegaPlayerController::RefreshBattleMenuAbilities()
{
    if (!BattleMenuWidget)
    {
        return;
    }

    AProjectVegaPlayerCharacter* PC = Cast<AProjectVegaPlayerCharacter>(GetPawn());
    if (!PC)
    {
        return;
    }

    BattleMenuWidget->SetAbilities(PC->GetAvailableAbilities());
}

void AProjectVegaPlayerController::UpdateTargetingPreview()
{
    if (!SelectedAbility || !GetWorld())
    {
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(42, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Selected: %s"), *SelectedAbility->AbilityName.ToString()));
    }

    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        return;
    }

    const FVector HitLocation = Hit.Location;
    DrawDebugLine(GetWorld(), PlayerCameraManager->GetCameraLocation(), HitLocation, FColor::Cyan, false, 0.f, 0, 1.5f);
    DrawDebugSphere(GetWorld(), HitLocation, 24.f, 12, FColor::Cyan, false, 0.f, 0, 1.5f);

    if (AActor* HoveredActor = Hit.GetActor())
    {
        FVector Origin;
        FVector Extents;
        HoveredActor->GetActorBounds(true, Origin, Extents);
        DrawDebugBox(GetWorld(), Origin, Extents, FQuat::Identity, FColor::Cyan, false, 0.f, 0, 1.5f);
        DrawDebugString(GetWorld(), Origin + FVector(0.f, 0.f, Extents.Z + 20.f),
            FString::Printf(TEXT("Target: %s"), *HoveredActor->GetName()), nullptr, FColor::Cyan, 0.f, true);
    }

    switch (SelectedAbility->TargetingMode)
    {
        case EAbilityTargetingMode::Splash:
            if (SelectedAbility->SplashRadius > KINDA_SMALL_NUMBER)
            {
                DrawDebugSphere(GetWorld(), HitLocation, SelectedAbility->SplashRadius, 24, FColor::Orange, false, 0.f, 0, 1.2f);
            }
            break;
        case EAbilityTargetingMode::AllEnemies:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Yellow, TEXT("Targeting: All Enemies"));
            }
            break;
        case EAbilityTargetingMode::AllAllies:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Green, TEXT("Targeting: All Allies"));
            }
            break;
        case EAbilityTargetingMode::Everyone:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Silver, TEXT("Targeting: Everyone"));
            }
            break;
        case EAbilityTargetingMode::RandomEnemies:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Yellow,
                    FString::Printf(TEXT("Targeting: Random Enemies (%d)"), FMath::Max(1, SelectedAbility->RandomTargetCount)));
            }
            break;
        case EAbilityTargetingMode::RandomAllies:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Green,
                    FString::Printf(TEXT("Targeting: Random Allies (%d)"), FMath::Max(1, SelectedAbility->RandomTargetCount)));
            }
            break;
        case EAbilityTargetingMode::LowestHealthEnemy:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Yellow, TEXT("Targeting: Lowest Health Enemy"));
            }
            break;
        case EAbilityTargetingMode::LowestHealthAlly:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Green, TEXT("Targeting: Lowest Health Ally"));
            }
            break;
        case EAbilityTargetingMode::HighestHealthEnemy:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Yellow, TEXT("Targeting: Highest Health Enemy"));
            }
            break;
        case EAbilityTargetingMode::HighestHealthAlly:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(43, 0.f, FColor::Green, TEXT("Targeting: Highest Health Ally"));
            }
            break;
        default:
            break;
    }
}
