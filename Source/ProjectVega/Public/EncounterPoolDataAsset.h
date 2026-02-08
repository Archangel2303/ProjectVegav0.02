#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FloorMapTypes.h"
#include "EncounterPoolDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTVEGA_API UEncounterPoolDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FWeightedEncounterEntry> Encounters;

    UFUNCTION(BlueprintCallable, Category="Encounter")
    UEncounterDefinitionDataAsset* PickEncounter(int32 Seed = 0) const;
};
