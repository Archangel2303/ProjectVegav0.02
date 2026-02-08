#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AbilityDataAsset.h"
#include "AbilityTypes.h"
#include "AbilitySystemSubsystem.generated.h"

UCLASS()
class PROJECTVEGA_API UAbilitySystemSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Ability System")
    TArray<FGuid> ExecuteAbility(UAbilityDataAsset* Ability, const FAbilityContext& Context);
};
