#include "AITurnComponent.h"
#include "AbilityExecutorComponent.h"
#include "AbilityDataAsset.h"
#include "TurnManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "ProjectVegaAIPawn.h"

UAITurnComponent::UAITurnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UAITurnComponent::PerformAITurn(AActor* TargetOverride)
{
    if (!GetOwner())
    {
        return false;
    }

    UAbilityExecutorComponent* Executor = GetOwner()->FindComponentByClass<UAbilityExecutorComponent>();
    if (!Executor)
    {
        return false;
    }

    UAbilityDataAsset* ChosenAbility = DefaultAbility;
    if (bUseAbilityLoadout)
    {
        if (const AProjectVegaAIPawn* Pawn = Cast<AProjectVegaAIPawn>(GetOwner()))
        {
            const TArray<UAbilityDataAsset*>& Loadout = Pawn->GetAbilityLoadout();
            if (Loadout.Num() > 0)
            {
                const int32 Index = bRandomizeAbility ? FMath::RandRange(0, Loadout.Num() - 1) : 0;
                ChosenAbility = Loadout[Index];
            }
        }
    }

    if (!ChosenAbility)
    {
        return false;
    }

    TArray<AActor*> Targets;
    if (TargetOverride)
    {
        Targets.Add(TargetOverride);
    }
    else if (bTargetSelfIfNoTarget)
    {
        Targets.Add(GetOwner());
    }

    Executor->ExecuteAbilityByTargets(ChosenAbility, Targets, 1.f);

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

    return true;
}
