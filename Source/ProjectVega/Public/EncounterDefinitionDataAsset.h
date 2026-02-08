#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FloorMapTypes.h"
#include "EncounterDefinitionDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTVEGA_API UEncounterDefinitionDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName EncounterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEncounterDifficulty Difficulty = EEncounterDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FEnemySpawnEntry> Enemies;
};
