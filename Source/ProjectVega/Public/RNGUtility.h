#pragma once

#include "CoreMinimal.h"
#include "RNGUtility.generated.h"

UCLASS()
class PROJECTVEGA_API URNGUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="RNG")
    static int32 MakeDeterministicSeed(int32 BaseSeed, int32 Salt);
};
