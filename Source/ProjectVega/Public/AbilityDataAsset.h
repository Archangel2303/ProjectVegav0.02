#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilityTypes.h"
#include "AbilityDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTVEGA_API UAbilityDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FEffectPayload> Effects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Cooldown = 0.f;

    /** Targeting: radius (0 = none), max targets (0 = unlimited), or target self */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TargetingRadius = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxTargets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bTargetSelf = false;

    /** Optional gameplay tag requirements (caster must have these tags) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer RequirementTags;

    /** Optional target filtering by Gameplay Tags. If non-empty, only actors
     *  with matching tags will be considered targets. Use `bRequireAllTargetTags`
     *  to require all tags (AND) or default to any matching tag (OR).
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer TargetFilterTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRequireAllTargetTags = false;

    // Costs
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FocusCost = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float HealthCost = 0.f; // flat health cost

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SoftcapPercentGained = 0.f; // percent of MaxHealth related to soft-cap exchange (0..1)

    /** Minimum remaining HP as percent of MaxHealth after using ability. If the caster's
     *  resulting health would be below this, the ability cannot be used. 0..1
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinHealthPercentAfterUse = 0.05f;

    /** Cooldown in turns (0 = no cooldown) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 CooldownTurns = 0;
};
