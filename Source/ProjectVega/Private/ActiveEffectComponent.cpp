#include "ActiveEffectComponent.h"
#include "TurnManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UActiveEffectComponent::UActiveEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UActiveEffectComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->OnTurnAdvanced.AddDynamic(this, &UActiveEffectComponent::HandleTurnAdvanced);
            }
        }
    }
}

void UActiveEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->OnTurnAdvanced.RemoveDynamic(this, &UActiveEffectComponent::HandleTurnAdvanced);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}

FGuid UActiveEffectComponent::AddActiveEffect(const FEffectPayload& Payload, const FGuid& SourceId, int32 InitialDuration)
{
    FActiveEffectInstance Inst;
    Inst.Payload = Payload;
    Inst.SourceId = SourceId;
    Inst.RemainingDuration = InitialDuration;
    Inst.StackCount = 1;
    Inst.InstanceId = FGuid::NewGuid();
    ActiveEffects.Add(Inst);
    return Inst.InstanceId;
}

int32 UActiveEffectComponent::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

bool UActiveEffectComponent::GetActiveEffectInstanceIdAt(int32 Index, FGuid& OutId) const
{
    if (!ActiveEffects.IsValidIndex(Index))
    {
        return false;
    }

    OutId = ActiveEffects[Index].InstanceId;
    return true;
}

bool UActiveEffectComponent::RemoveActiveEffectById(const FGuid& InstanceId)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].InstanceId == InstanceId)
        {
            ActiveEffects.RemoveAt(i);
            return true;
        }
    }
    return false;
}

void UActiveEffectComponent::TickTurn()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FActiveEffectInstance& Inst = ActiveEffects[i];
        if (Inst.RemainingDuration > 0)
        {
            Inst.RemainingDuration--;
            if (Inst.RemainingDuration <= 0)
            {
                ActiveEffects.RemoveAt(i);
            }
        }
    }
}

void UActiveEffectComponent::HandleTurnAdvanced(int32 TurnNumber)
{
    TickTurn();
}
