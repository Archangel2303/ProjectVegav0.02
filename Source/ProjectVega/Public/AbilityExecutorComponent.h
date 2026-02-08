#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityDataAsset.h"
#include "AbilityTypes.h"
#include "AbilityExecutorComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UAbilityExecutorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Abilities")
    void ExecuteAbilityByTargets(UAbilityDataAsset* Ability, const TArray<AActor*>& Targets, float Magnifier = 1.f);

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void ExecuteAbilityWithContext(UAbilityDataAsset* Ability, const FAbilityContext& Context);
};
