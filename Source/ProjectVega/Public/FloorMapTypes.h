#pragma once

#include "CoreMinimal.h"
#include "FloorMapTypes.generated.h"

class UEncounterDefinitionDataAsset;

UENUM(BlueprintType)
enum class EEncounterDifficulty : uint8
{
    Easy,
    Medium,
    Hard
};

UENUM(BlueprintType)
enum class EFloorNodeType : uint8
{
    Normal,
    MiniBoss,
    Boss
};

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<APawn> EnemyClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FWeightedEncounterEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterDefinitionDataAsset* Encounter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight = 1.f;
};

USTRUCT(BlueprintType)
struct FFloorMapNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 NodeId = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFloorNodeType NodeType = EFloorNodeType::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEncounterDifficulty Difficulty = EEncounterDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterDefinitionDataAsset* Encounter = nullptr;
};
