#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FloorMapTypes.h"
#include "FloorDefinitionDataAsset.generated.h"

class UEncounterPoolDataAsset;

UCLASS(BlueprintType)
class PROJECTVEGA_API UFloorDefinitionDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName EncounterMapName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Rows = 15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Columns = 7;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinNodesPerRow = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxNodesPerRow = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 NumMiniBossNodes = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EasyWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MediumWeight = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float HardWeight = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CombatNodeWeight = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EliteNodeWeight = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RestNodeWeight = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ShopNodeWeight = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EventNodeWeight = 0.13f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterPoolDataAsset* EasyPool = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterPoolDataAsset* MediumPool = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterPoolDataAsset* HardPool = nullptr;


    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterPoolDataAsset* MiniBossPool = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UEncounterPoolDataAsset* BossPool = nullptr;
};
