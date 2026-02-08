#include "AugmentComponentExtensions.h"
#include "AugmentComponent.h"
#include "AugmentDataAsset.h"
#include "AttributeComponent.h"

TArray<UAbilityDataAsset*> UAugmentComponentExtensions::GetGrantedAbilities(UAugmentComponent* Comp)
{
    TArray<UAbilityDataAsset*> Out;
    if (!Comp || !Comp->GetOwner()) return Out;

    UAttributeComponent* Attr = Comp->GetOwner()->FindComponentByClass<UAttributeComponent>();

    for (UAugmentDataAsset* Aug : Comp->GetEquippedAugments())
    {
        if (!Aug) continue;

        // Add base active abilities
        for (UAbilityDataAsset* A : Aug->ActiveAbilities)
        {
            if (A) Out.AddUnique(A);
        }

        // Overdrive: requires SoftcapPercent >= threshold
        if (Aug->OverdriveAbility && Attr)
        {
            float Soft = Attr->GetAttribute(TEXT("SoftcapPercent"));
            if (Soft >= Aug->OverdriveSoftcapThreshold)
            {
                Out.AddUnique(Aug->OverdriveAbility);
            }
        }

        // Herculean: requires effective max health >= threshold
        if (Aug->HerculeanAbility && Attr)
        {
            float MaxEff = Attr->GetAttribute(TEXT("MaxHealth"));
            if (MaxEff >= Aug->HerculeanHPThreshold)
            {
                Out.AddUnique(Aug->HerculeanAbility);
            }
        }
    }

    return Out;
}
