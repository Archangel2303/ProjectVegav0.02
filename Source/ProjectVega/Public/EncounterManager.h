#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterManager.generated.h"

class UProjectVegaRunStateSubsystem;
class UEncounterDefinitionDataAsset;
class AProjectVegaPlayerCharacter;
class UAITurnComponent;
class USceneComponent;

UCLASS()
class PROJECTVEGA_API AEncounterManager : public AActor
{
    GENERATED_BODY()

public:
    AEncounterManager();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    USceneComponent* PlayerSpawnRoot = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    USceneComponent* EnemySpawnRoot = nullptr;

    UFUNCTION(BlueprintCallable, Category="Encounter")
    void StartEncounterFromRunState();

protected:
    virtual void BeginPlay() override;

private:
    void SpawnEnemies(UEncounterDefinitionDataAsset* Encounter);
    void EnsurePlayerSpawned();
    void BindTurnManager();
    void HandleTurnAdvanced(int32 TurnNumber);
    void ExecuteAITurn();
    AActor* SelectAITarget() const;
    float ScoreTarget(AActor* Target) const;
    void CollectSpawnTransforms(const USceneComponent* Root, TArray<FTransform>& OutTransforms) const;

    UPROPERTY()
    AProjectVegaPlayerCharacter* PlayerCharacter = nullptr;

    UPROPERTY()
    TArray<AActor*> SpawnedEnemies;

    UPROPERTY()
    int32 NextEnemyIndex = 0;

    UPROPERTY()
    bool bPlayerTurn = true;
};
