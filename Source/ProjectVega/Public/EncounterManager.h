#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterManager.generated.h"

class UProjectVegaRunStateSubsystem;
class UEncounterDefinitionDataAsset;
class AProjectVegaPlayerCharacter;
class UAITurnComponent;
class USceneComponent;
class UProjectVegaDeathWidget;
class UProjectVegaRewardWidget;
class UAugmentDataAsset;

UCLASS()
class PROJECTVEGA_API AEncounterManager : public AActor
{
    GENERATED_BODY()

public:
    AEncounterManager();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    bool bLogEncounter = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    bool bLogAITargeting = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    TSubclassOf<UProjectVegaDeathWidget> DeathWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    TSubclassOf<UProjectVegaRewardWidget> RewardWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    USceneComponent* PlayerSpawnRoot = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter")
    USceneComponent* EnemySpawnRoot = nullptr;

    UFUNCTION(BlueprintCallable, Category="Encounter")
    void StartEncounterFromRunState();

    UFUNCTION(BlueprintCallable, Category="Encounter")
    void ResolveEncounterDebug(bool bSkipRewards = false);

protected:
    virtual void BeginPlay() override;

private:
    void SpawnEnemies(UEncounterDefinitionDataAsset* Encounter);
    void EnsurePlayerSpawned();
    void BindTurnManager();
    void HandleTurnAdvanced(int32 TurnNumber);
    void ExecuteAITurn();
    bool CheckEncounterEndState();
    bool IsActorDefeated(AActor* Actor) const;
    void ShowDeathPrompt();
    void ShowRewardPrompt();
    UFUNCTION()
    void HandleDeathRetry();

    UFUNCTION()
    void HandleDeathQuit();

    UFUNCTION()
    void HandleRewardNanites(int32 Amount);

    UFUNCTION()
    void HandleRewardAugment(UAugmentDataAsset* Augment);

    UFUNCTION()
    void HandleRewardContinue();

    UFUNCTION()
    void HandleRewardVialOpened();
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

    UPROPERTY()
    bool bEncounterEnded = false;

    UPROPERTY()
    UProjectVegaRewardWidget* ActiveRewardWidget = nullptr;
};
