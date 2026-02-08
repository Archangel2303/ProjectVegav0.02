#include "AbilitySystemSubsystem.h"
#include "AbilityDataAsset.h"
#include "AbilityTypes.h"
#include "AttributeComponent.h"
#include "ActiveEffectComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagAssetInterface.h"
#include "AbilityCooldownComponent.h"

TArray<FGuid> UAbilitySystemSubsystem::ExecuteAbility(UAbilityDataAsset* Ability, const FAbilityContext& Context)
{
    TArray<FGuid> CreatedEffectIds;

    if (!Ability)
    {
        return CreatedEffectIds;
    }

    // Requirement checks: if Ability has RequirementTags, ensure caster satisfies them
    if (!Ability->RequirementTags.IsEmpty())
    {
        if (!Context.Caster)
        {
            return CreatedEffectIds;
        }

        if (Context.Caster->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
        {
            FGameplayTagContainer OwnedTags;
            if (IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Context.Caster))
            {
                TagIface->GetOwnedGameplayTags(OwnedTags);
            }
            if (!OwnedTags.HasAll(Ability->RequirementTags))
            {
                return CreatedEffectIds;
            }
        }
        else
        {
            return CreatedEffectIds;
        }
    }

    // Resolve targets if none provided in context
    FAbilityContext ResolvedContext = Context; // mutable copy
    if (ResolvedContext.Targets.Num() == 0)
    {
        if (Ability->bTargetSelf && Context.Caster)
        {
            bool bAccept = true;
            if (!Ability->TargetFilterTags.IsEmpty())
            {
                bAccept = false;
                if (Context.Caster->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
                {
                    FGameplayTagContainer OwnedTags;
                    if (IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Context.Caster))
                    {
                        TagIface->GetOwnedGameplayTags(OwnedTags);
                    }
                    if (Ability->bRequireAllTargetTags)
                    {
                        bAccept = OwnedTags.HasAll(Ability->TargetFilterTags);
                    }
                    else
                    {
                        bAccept = OwnedTags.HasAny(Ability->TargetFilterTags);
                    }
                }
            }

            if (bAccept)
            {
                ResolvedContext.Targets.Add(Context.Caster);
            }
        }
        else if (Ability->TargetingRadius > KINDA_SMALL_NUMBER && Context.Caster)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                TArray<AActor*> AllActors;
                UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

                FVector Origin = Context.Caster->GetActorLocation();
                for (AActor* Candidate : AllActors)
                {
                    if (!Candidate || Candidate == Context.Caster) continue;

                    float Dist2 = FVector::DistSquared(Origin, Candidate->GetActorLocation());
                    if (Dist2 <= FMath::Square(Ability->TargetingRadius))
                    {
                        bool bPassFilter = true;
                        if (!Ability->TargetFilterTags.IsEmpty())
                        {
                            bPassFilter = false;
                            if (Candidate->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
                            {
                                FGameplayTagContainer OwnedTags;
                                if (IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Candidate))
                                {
                                    TagIface->GetOwnedGameplayTags(OwnedTags);
                                }
                                if (Ability->bRequireAllTargetTags)
                                {
                                    bPassFilter = OwnedTags.HasAll(Ability->TargetFilterTags);
                                }
                                else
                                {
                                    bPassFilter = OwnedTags.HasAny(Ability->TargetFilterTags);
                                }
                            }
                        }

                        if (bPassFilter)
                        {
                            ResolvedContext.Targets.Add(Candidate);
                            if (Ability->MaxTargets > 0 && ResolvedContext.Targets.Num() >= Ability->MaxTargets)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (ResolvedContext.Targets.Num() == 0)
    {
        return CreatedEffectIds;
    }

    // Attribute-cost & Cooldown checks
    if (Context.Caster)
    {
        UAbilityCooldownComponent* CooldownComp = Context.Caster->FindComponentByClass<UAbilityCooldownComponent>();
        if (CooldownComp && CooldownComp->IsOnCooldown(Ability->AbilityName))
        {
            return CreatedEffectIds;
        }

        UAttributeComponent* CasterAttr = Context.Caster->FindComponentByClass<UAttributeComponent>();
        if (!CasterAttr)
        {
            return CreatedEffectIds;
        }

        if (Ability->FocusCost > 0.f)
        {
            float Focus = CasterAttr->GetAttribute(TEXT("Focus"));
            if (Focus < Ability->FocusCost)
            {
                return CreatedEffectIds;
            }
        }

        float MaxHealth = CasterAttr->GetAttribute(TEXT("MaxHealth"));
        float CurrHealth = CasterAttr->GetAttribute(TEXT("Health"));
        if (MaxHealth <= 0.f) MaxHealth = CurrHealth > 0.f ? CurrHealth : 1.f;

        float HealthCostTotal = Ability->HealthCost;
        if (HealthCostTotal > 0.f)
        {
            float MinAllowed = MaxHealth * Ability->MinHealthPercentAfterUse;
            if ((CurrHealth - HealthCostTotal) < MinAllowed)
            {
                return CreatedEffectIds;
            }
        }
    }

    // Execute payloads
    for (const FEffectPayload& Payload : Ability->Effects)
    {
        switch (Payload.EffectType)
        {
            case EEffectType::Damage:
            {
                float Base = Payload.DamageParams.Amount;
                float Amount = Base * ResolvedContext.Magnifier;

                for (AActor* Target : ResolvedContext.Targets)
                {
                    if (!Target) continue;

                    UAttributeComponent* Attr = Target->FindComponentByClass<UAttributeComponent>();
                    if (Attr)
                    {
                        Attr->ApplyAttributeDelta(TEXT("Health"), -Amount);
                    }

                    if (Payload.Duration > 0)
                    {
                        UActiveEffectComponent* ActiveComp = Target->FindComponentByClass<UActiveEffectComponent>();
                        if (ActiveComp)
                        {
                            FGuid SourceId = FGuid::NewGuid();
                            FGuid InstanceId = ActiveComp->AddActiveEffect(Payload, SourceId, Payload.Duration);
                            CreatedEffectIds.Add(InstanceId);
                        }
                    }
                }
            }
            break;

            default:
                break;
        }
    }

    // Apply caster-side costs and softcap/ cooldown
    if (Context.Caster)
    {
        UAttributeComponent* CasterAttr = Context.Caster->FindComponentByClass<UAttributeComponent>();
        if (CasterAttr)
        {
            if (Ability->FocusCost > 0.f)
            {
                CasterAttr->ApplyAttributeDelta(TEXT("Focus"), -Ability->FocusCost);
            }

            float HealthCostTotal = Ability->HealthCost;
            if (HealthCostTotal > 0.f)
            {
                CasterAttr->ApplyAttributeDelta(TEXT("Health"), -HealthCostTotal);
            }

            // Apply soft-cap percent gained as attribute change
            if (Ability->SoftcapPercentGained > 0.f)
            {
                CasterAttr->ApplyAttributeDelta(TEXT("SoftcapPercent"), Ability->SoftcapPercentGained);
            }
        }

        UAbilityCooldownComponent* CooldownComp = Context.Caster->FindComponentByClass<UAbilityCooldownComponent>();
        if (CooldownComp && Ability->CooldownTurns > 0)
        {
            CooldownComp->SetCooldown(Ability->AbilityName, Ability->CooldownTurns);
        }
    }

    return CreatedEffectIds;
}
