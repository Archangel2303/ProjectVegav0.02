#pragma once

#include "CoreMinimal.h"
#include "AugmentComponent.h"
#include "AugmentComponentExtensions.generated.h"

// Convenience header to expose helper signatures to tests/blueprints

UCLASS()
class PROJECTVEGA_API UAugmentComponentExtensions : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Augments")
    static TArray<UAbilityDataAsset*> GetGrantedAbilities(UAugmentComponent* Comp);
};
