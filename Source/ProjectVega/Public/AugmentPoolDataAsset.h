#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AugmentPoolDataAsset.generated.h"

class UAugmentDataAsset;

USTRUCT(BlueprintType)
struct FAugmentPoolEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UAugmentDataAsset* Augment = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight = 1.f;
};

UCLASS(BlueprintType)
class PROJECTVEGA_API UAugmentPoolDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FAugmentPoolEntry> Augments;

    UFUNCTION(BlueprintCallable, Category="Augments")
    TArray<UAugmentDataAsset*> PickAugments(int32 Count, int32 Seed = 0) const;
};
