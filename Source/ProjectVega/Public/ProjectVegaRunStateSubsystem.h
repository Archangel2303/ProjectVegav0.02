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
class UAugmentPoolDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFloorCleared);

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RunState")
    bool bLogFloorGeneration = false;

    UFUNCTION(BlueprintCallable, Category="RunState")
    void GenerateFloorMap(UFloorDefinitionDataAsset* FloorDefinition, int32 Seed = 0);

    UFUNCTION(BlueprintCallable, Category="RunState")
    const TArray<FFloorMapNode>& GetFloorNodes() const { return FloorNodes; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    bool IsNodeSelectable(int32 NodeId) const;

    UFUNCTION(BlueprintCallable, Category="RunState")
    TArray<int32> GetSelectableNodeIds() const;

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
    FName GetFloorMapName() const;

    UFUNCTION(BlueprintCallable, Category="RunState")
    FName GetEncounterMapName() const;

    UFUNCTION(BlueprintCallable, Category="RunState")
    void CacheFloorMapName(FName MapName);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void CacheEncounterMapName(FName MapName);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void CapturePlayerLoadout(AProjectVegaPlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void ApplyLoadout(AProjectVegaPlayerCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category="RunState")
    void DumpFloorMapToLog() const;

    UFUNCTION(BlueprintCallable, Category="RunState")
    int32 GetNanites() const { return Nanites; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    void AddNanites(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="RunState")
    const TArray<UAugmentDataAsset*>& GetAugmentInventory() const { return AugmentInventory; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    void AddAugmentToInventory(UAugmentDataAsset* Augment);

    UFUNCTION(BlueprintCallable, Category="RunState")
    TArray<UAugmentDataAsset*> RollRewardAugments(int32 Count);

    UFUNCTION(BlueprintCallable, Category="RunState")
    int32 RollRewardNanites();

    UFUNCTION(BlueprintCallable, Category="RunState")
    bool IsFloorCleared() const { return bFloorCleared; }

    UFUNCTION(BlueprintCallable, Category="RunState")
    void AdvanceToNextFloor();

    UPROPERTY(BlueprintAssignable, Category="RunState")
    FOnFloorCleared OnFloorCleared;

    UFUNCTION(BlueprintCallable, Category="RunState")
    void MarkEncounterResolved(bool bPlayerWon);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void ReturnToFloorMap(bool bRegenerate);

    UFUNCTION(BlueprintCallable, Category="RunState")
    void ResetRun(bool bRegenerate);

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
    int32 CurrentNodeId = INDEX_NONE;

    UPROPERTY()
    int32 LastSeed = 0;

    UPROPERTY()
    bool bHasGeneratedMap = false;

    UPROPERTY()
    bool bPendingRegenerate = false;

    UPROPERTY()
    UEncounterDefinitionDataAsset* SelectedEncounter = nullptr;

    UPROPERTY()
    FPlayerLoadout PlayerLoadout;

    UPROPERTY()
    int32 Nanites = 0;

    UPROPERTY()
    TArray<UAugmentDataAsset*> AugmentInventory;

    UPROPERTY()
    int32 CurrentFloorNumber = 1;

    UPROPERTY()
    bool bFloorCleared = false;

    UPROPERTY()
    FName CachedFloorMapName;

    UPROPERTY()
    FName CachedEncounterMapName;
};
