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
    Combat,
    Elite,
    Rest,
    Shop,
    Event,
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
    int32 Row = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Column = 0;

    // Normalized 0..1 position in the map widget
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector2D MapPosition = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFloorNodeType NodeType = EFloorNodeType::Combat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEncounterDifficulty Difficulty = EEncounterDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterDefinitionDataAsset* Encounter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<int32> LinkedNodeIds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bVisited = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCleared = false;
};
