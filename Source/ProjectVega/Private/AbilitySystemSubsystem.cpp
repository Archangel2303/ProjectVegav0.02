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

    // Resolve targets and apply targeting mode rules
    FAbilityContext ResolvedContext = Context; // mutable copy
    UWorld* World = GetWorld();

    auto PassesTargetFilter = [&](AActor* Candidate) -> bool
    {
        if (!Candidate) return false;
        if (Ability->TargetFilterTags.IsEmpty()) return true;

        if (Candidate->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
        {
            FGameplayTagContainer OwnedTags;
            if (IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Candidate))
            {
                TagIface->GetOwnedGameplayTags(OwnedTags);
            }
            return Ability->bRequireAllTargetTags ? OwnedTags.HasAll(Ability->TargetFilterTags)
                                                   : OwnedTags.HasAny(Ability->TargetFilterTags);
        }

        return false;
    };

    auto CollectAllActors = [&]() -> TArray<AActor*>
    {
        TArray<AActor*> Out;
        if (World)
        {
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Out);
        }
        return Out;
    };

    auto ApplyMaxTargets = [&]()
    {
        if (Ability->MaxTargets > 0 && ResolvedContext.Targets.Num() > Ability->MaxTargets)
        {
            ResolvedContext.Targets.SetNum(Ability->MaxTargets);
        }
    };

    // If nothing selected yet, use default single-target or self/radius rules
    if (ResolvedContext.Targets.Num() == 0)
    {
        if (Ability->bTargetSelf && Context.Caster)
        {
            if (PassesTargetFilter(Context.Caster))
            {
                ResolvedContext.Targets.Add(Context.Caster);
            }
        }
        else if (Ability->TargetingRadius > KINDA_SMALL_NUMBER && Context.Caster)
        {
            const FVector Origin = Context.Caster->GetActorLocation();
            for (AActor* Candidate : CollectAllActors())
            {
                if (!Candidate || Candidate == Context.Caster) continue;
                const float Dist2 = FVector::DistSquared(Origin, Candidate->GetActorLocation());
                if (Dist2 <= FMath::Square(Ability->TargetingRadius) && PassesTargetFilter(Candidate))
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

    // Apply targeting modes (multi-target, splash, random, all)
    switch (Ability->TargetingMode)
    {
        case EAbilityTargetingMode::Single:
            break;

        case EAbilityTargetingMode::Splash:
        {
            if (ResolvedContext.Targets.Num() > 0 && Ability->SplashRadius > KINDA_SMALL_NUMBER)
            {
                AActor* Primary = ResolvedContext.Targets[0];
                const FVector Origin = Primary->GetActorLocation();
                for (AActor* Candidate : CollectAllActors())
                {
                    if (!Candidate) continue;
                    const float Dist2 = FVector::DistSquared(Origin, Candidate->GetActorLocation());
                    if (Dist2 <= FMath::Square(Ability->SplashRadius) && PassesTargetFilter(Candidate))
                    {
                        ResolvedContext.Targets.AddUnique(Candidate);
                    }
                }
            }
        }
        break;

        case EAbilityTargetingMode::AllEnemies:
        case EAbilityTargetingMode::AllAllies:
        case EAbilityTargetingMode::Everyone:
        {
            ResolvedContext.Targets.Reset();
            for (AActor* Candidate : CollectAllActors())
            {
                if (!Candidate) continue;
                if (Ability->TargetingMode != EAbilityTargetingMode::Everyone && Candidate == Context.Caster)
                {
                    continue;
                }
                if (PassesTargetFilter(Candidate))
                {
                    ResolvedContext.Targets.Add(Candidate);
                }
            }

            if (Ability->TargetingMode == EAbilityTargetingMode::AllAllies && Ability->bIncludeCasterInAllies && Context.Caster)
            {
                ResolvedContext.Targets.AddUnique(Context.Caster);
            }
        }
        break;

        case EAbilityTargetingMode::RandomEnemies:
        case EAbilityTargetingMode::RandomAllies:
        {
            TArray<AActor*> Candidates;
            for (AActor* Candidate : CollectAllActors())
            {
                if (!Candidate) continue;
                if (Candidate == Context.Caster) continue;
                if (PassesTargetFilter(Candidate))
                {
                    Candidates.Add(Candidate);
                }
            }

            FRandomStream Stream(Context.Seed != 0 ? Context.Seed : FMath::Rand());
            for (int32 i = Candidates.Num() - 1; i > 0; --i)
            {
                int32 SwapIdx = Stream.RandRange(0, i);
                Candidates.Swap(i, SwapIdx);
            }

            ResolvedContext.Targets.Reset();
            int32 Count = FMath::Max(1, Ability->RandomTargetCount);
            for (int32 i = 0; i < Candidates.Num() && i < Count; ++i)
            {
                ResolvedContext.Targets.Add(Candidates[i]);
            }
        }
        break;
    }

    ApplyMaxTargets();

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
