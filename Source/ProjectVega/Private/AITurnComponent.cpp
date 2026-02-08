#include "AITurnComponent.h"
#include "AbilityExecutorComponent.h"
#include "AbilityDataAsset.h"
#include "TurnManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UAITurnComponent::UAITurnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UAITurnComponent::PerformAITurn(AActor* TargetOverride)
{
    if (!DefaultAbility || !GetOwner())
    {
        return false;
    }

    UAbilityExecutorComponent* Executor = GetOwner()->FindComponentByClass<UAbilityExecutorComponent>();
    if (!Executor)
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

    Executor->ExecuteAbilityByTargets(DefaultAbility, Targets, 1.f);

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
