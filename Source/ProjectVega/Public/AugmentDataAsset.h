#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AugmentTypes.h"
#include "AbilityDataAsset.h"
#include "AugmentDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTVEGA_API UAugmentDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Which slot this augment fits into
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EAugmentSlot Slot = EAugmentSlot::Core;

    // 1-2 active abilities granted by this augment
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<UAbilityDataAsset*> ActiveAbilities;

    // 0-1 extra passive ability (applied as ActiveEffect on equip)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAbilityDataAsset* ExtraPassiveAbility = nullptr;

    // Softcap percent to apply when equipping (0..1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EquipSoftcapPercent = 0.f;

    // Optional powerful abilities unlocked by thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAbilityDataAsset* OverdriveAbility = nullptr; // enabled when softcap >= OverdriveSoftcapThreshold

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float OverdriveSoftcapThreshold = 0.60f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAbilityDataAsset* HerculeanAbility = nullptr; // enabled when max HP >= HerculeanHPThreshold

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float HerculeanHPThreshold = 160.f;
};
