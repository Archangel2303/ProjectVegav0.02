#include "AugmentComponent.h"
#include "AbilitySystemSubsystem.h"
#include "AttributeComponent.h"
#include "ActiveEffectComponent.h"
#include "AbilityExecutorComponent.h"
#include "AbilityCooldownComponent.h"
#include "GameFramework/Actor.h"

UAugmentComponent::UAugmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAugmentComponent::BeginPlay()
{
    Super::BeginPlay();
}

const UAugmentDataAsset* UAugmentComponent::GetAugmentInSlot(EAugmentSlot Slot) const
{
    if (const UAugmentDataAsset* const* Found = Equipped.Find(Slot))
    {
        return *Found;
    }
    return nullptr;
}

TArray<UAugmentDataAsset*> UAugmentComponent::GetEquippedAugments() const
{
    TArray<UAugmentDataAsset*> Out;
    for (const auto& Pair : Equipped)
    {
        if (Pair.Value) Out.Add(Pair.Value);
    }
    return Out;
}

void UAugmentComponent::ClearAllAugments()
{
    TArray<EAugmentSlot> Slots;
    Equipped.GetKeys(Slots);
    for (EAugmentSlot Slot : Slots)
    {
        UnequipAugment(Slot);
    }
}

bool UAugmentComponent::EquipAugment(UAugmentDataAsset* Augment)
{
    if (!Augment || !GetOwner()) return false;

    // Check slot availability (replace if present)
    EAugmentSlot Slot = Augment->Slot;

    // If there's already an augment in this slot, unequip it first to remove its passive effects
    if (Equipped.Contains(Slot))
    {
        UnequipAugment(Slot);
    }

    // Check softcap cost: must be able to accept increase (we allow applying softcap)
    UAttributeComponent* Attr = GetOwner()->FindComponentByClass<UAttributeComponent>();
    if (!Attr) return false;

    // Applying softcap percent will immediately clamp health if it reduces max; allow equip
    // (If you want to forbid equipping when health would be clamped below min, add check here.)

    // Apply softcap percent increase
    if (Augment->EquipSoftcapPercent > 0.f)
    {
        Attr->ApplyAttributeDelta(TEXT("SoftcapPercent"), Augment->EquipSoftcapPercent);
    }

    // Grant passive extra ability as persistent ActiveEffect
    if (Augment->ExtraPassiveAbility)
    {
        UAbilitySystemSubsystem* Sub = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UAbilitySystemSubsystem>() : nullptr;
        if (Sub)
        {
            FAbilityContext Ctx;
            Ctx.Caster = GetOwner();
            // Using Ctx.Targets as [owner] to apply passive effects to owner
            Ctx.Targets.Add(GetOwner());
            // Execute passive ability: for persistent buffs, their Payload.Duration will be used
            TArray<FGuid> Created = Sub->ExecuteAbility(Augment->ExtraPassiveAbility, Ctx);

            if (Created.Num() > 0)
            {
                EquippedPassiveEffectIds.Add(Slot, Created);
            }
        }
    }

    // Register active abilities by simply storing the augment; execution can query AugmentComponent for abilities
    Equipped.Add(Slot, Augment);

    return true;
}

bool UAugmentComponent::UnequipAugment(EAugmentSlot Slot)
{
    UAugmentDataAsset** Found = Equipped.Find(Slot);
    if (!Found || !*Found) return false;

    UAugmentDataAsset* Aug = *Found;

    // Remove passive effects that were recorded when equipping
    if (EquippedPassiveEffectIds.Contains(Slot))
    {
        TArray<FGuid> Ids = EquippedPassiveEffectIds[Slot];
        UActiveEffectComponent* ActiveComp = GetOwner()->FindComponentByClass<UActiveEffectComponent>();
        if (ActiveComp)
        {
            for (const FGuid& Id : Ids)
            {
                ActiveComp->RemoveActiveEffectById(Id);
            }
        }
        EquippedPassiveEffectIds.Remove(Slot);
    }

    Equipped.Remove(Slot);

    // Reduce softcap percent applied by this augment
    if (Aug->EquipSoftcapPercent > 0.f)
    {
        UAttributeComponent* Attr = GetOwner()->FindComponentByClass<UAttributeComponent>();
        if (Attr)
        {
            Attr->ApplyAttributeDelta(TEXT("SoftcapPercent"), -Aug->EquipSoftcapPercent);
        }
    }

    return true;
}
