#include "AbilityCooldownComponent.h"
#include "TurnManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UAbilityCooldownComponent::UAbilityCooldownComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityCooldownComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->OnTurnAdvanced.AddDynamic(this, &UAbilityCooldownComponent::HandleTurnAdvanced);
            }
        }
    }
}

void UAbilityCooldownComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->OnTurnAdvanced.RemoveDynamic(this, &UAbilityCooldownComponent::HandleTurnAdvanced);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}

bool UAbilityCooldownComponent::IsOnCooldown(FName AbilityName) const
{
    const int32* Val = RemainingCooldowns.Find(AbilityName);
    return Val && *Val > 0;
}

void UAbilityCooldownComponent::SetCooldown(FName AbilityName, int32 Turns)
{
    if (Turns <= 0)
    {
        RemainingCooldowns.Remove(AbilityName);
    }
    else
    {
        RemainingCooldowns.Add(AbilityName, Turns);
    }
}

void UAbilityCooldownComponent::TickTurn()
{
    for (auto It = RemainingCooldowns.CreateIterator(); It; ++It)
    {
        int32& Val = It.Value();
        Val = FMath::Max(0, Val - 1);
        if (Val == 0)
        {
            It.RemoveCurrent();
        }
    }
}

void UAbilityCooldownComponent::HandleTurnAdvanced(int32 TurnNumber)
{
    TickTurn();
}
