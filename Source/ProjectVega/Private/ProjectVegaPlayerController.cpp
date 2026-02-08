#include "ProjectVegaPlayerController.h"
#include "TurnEndWidget.h"
#include "ProjectVegaBattleMenuWidget.h"
#include "ProjectVegaFloorMapWidget.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaPlayerCharacter.h"
#include "AbilityExecutorComponent.h"
#include "AbilityDataAsset.h"
#include "TurnManager.h"
#include "ProjectVegaRunStateSubsystem.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

AProjectVegaPlayerController::AProjectVegaPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    PrimaryActorTick.bCanEverTick = true;
    bShowFloorMapWidget = true;
}

void AProjectVegaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("BattleMenu: BeginPlay for %s"), *GetName());

    if (EndTurnWidgetClass)
    {
        EndTurnWidget = CreateWidget<UTurnEndWidget>(this, EndTurnWidgetClass);
        if (EndTurnWidget)
        {
            EndTurnWidget->AddToViewport();
        }
    }

    if (!bShowFloorMapWidget)
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
                BattleMenuWidget->AddToViewport();
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

    if (bShowFloorMapWidget)
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
                FloorMapWidget->AddToViewport();
                if (UGameInstance* GI = GetWorld()->GetGameInstance())
                {
                    if (UProjectVegaRunStateSubsystem* RunState = GI->GetSubsystem<UProjectVegaRunStateSubsystem>())
                    {
                        FloorMapWidget->SetRunState(RunState);
                    }
                }
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
        default:
            break;
    }
}
