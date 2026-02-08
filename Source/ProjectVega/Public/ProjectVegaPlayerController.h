#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectVegaPlayerController.generated.h"

class UTurnEndWidget;
class UProjectVegaBattleMenuWidget;
class UProjectVegaFloorMapWidget;
class UAbilityDataAsset;

UCLASS()
class PROJECTVEGA_API AProjectVegaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AProjectVegaPlayerController();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UTurnEndWidget> EndTurnWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UProjectVegaBattleMenuWidget> BattleMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UProjectVegaFloorMapWidget> FloorMapWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    bool bShowFloorMapWidget = false;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;

    UFUNCTION()
    void SelectAbilityIndex1();

    UFUNCTION()
    void SelectAbilityIndex2();

    UFUNCTION()
    void SelectAbilityIndex3();

    UFUNCTION()
    void SelectAbilityIndex4();

    UFUNCTION()
    void HandleTargetClick();

    UFUNCTION()
    void HandleAbilitySelected(UAbilityDataAsset* Ability);

    UFUNCTION()
    void HandleEndTurnRequested();

    void SelectAbilityByIndex(int32 Index);
    void RefreshBattleMenuAbilities();
    void UpdateTargetingPreview();

private:
    UPROPERTY()
    UTurnEndWidget* EndTurnWidget = nullptr;

    UPROPERTY()
    UProjectVegaBattleMenuWidget* BattleMenuWidget = nullptr;

    UPROPERTY()
    UProjectVegaFloorMapWidget* FloorMapWidget = nullptr;

    UPROPERTY()
    UAbilityDataAsset* SelectedAbility = nullptr;
};
