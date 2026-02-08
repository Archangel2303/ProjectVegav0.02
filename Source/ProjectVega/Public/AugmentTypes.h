#pragma once

#include "CoreMinimal.h"
#include "AugmentTypes.generated.h"

UENUM(BlueprintType)
enum class EAugmentSlot : uint8
{
    Head,
    Torso,
    LArm,
    RArm,
    LLeg,
    RLeg,
    Core,
    Extra1,
    Extra2,
    Extra3,
    Extra4
};
