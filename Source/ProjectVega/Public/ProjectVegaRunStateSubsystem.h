#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FloorMapTypes.h"
#include "ProjectVegaRunStateSubsystem.generated.h"

class UFloorDefinitionDataAsset;
class UEncounterDefinitionDataAsset;
class UEncounterPoolDataAsset;
class UAugmentDataAsset;
class UAbilityDataAsset;
class AProjectVegaPlayerCharacter;

USTRUCT(BlueprintType)
struct FPlayerLoadout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<UAugmentDataAsset*> EquippedAugments;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<UAbilityDataAsset*> DefaultAbilities;
};

UCLASS()
class PROJECTVEGA_API UProjectVegaRunStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="RunState")
    void GenerateFloorMap(UFloorDefinitionDataAsset* FloorDefinition, int32 Seed = 0);

    UFUNCTION(BlueprintCallable, Category="RunState")
    const TArray<FFloorMapNode>& GetFloorNodes() const { return FloorNodes; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    bool SelectNode(int32 NodeId);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void StartSelectedEncounter();

    UFUNCTION(BlueprintCallable, Category="RunState")
    UEncounterDefinitionDataAsset* GetSelectedEncounter() const { return SelectedEncounter; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    void SetPlayerLoadout(const FPlayerLoadout& Loadout) { PlayerLoadout = Loadout; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    const FPlayerLoadout& GetPlayerLoadout() const { return PlayerLoadout; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    void CapturePlayerLoadout(AProjectVegaPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void ApplyLoadout(AProjectVegaPlayerCharacter* Player) const;

private:
    UEncounterDefinitionDataAsset* PickEncounterFromPool(UEncounterPoolDataAsset* Pool, FRandomStream& Stream) const;
    EEncounterDifficulty RollNormalDifficulty(FRandomStream& Stream) const;

    UPROPERTY()
    UFloorDefinitionDataAsset* CurrentFloor = nullptr;

    UPROPERTY()
    TArray<FFloorMapNode> FloorNodes;

    UPROPERTY()
    int32 SelectedNodeId = INDEX_NONE;

    UPROPERTY()
    UEncounterDefinitionDataAsset* SelectedEncounter = nullptr;

    UPROPERTY()
    FPlayerLoadout PlayerLoadout;
};
