#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaBattleMenuWidget.generated.h"

class UAbilityDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleAbilitySelected, UAbilityDataAsset*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBattleEndTurnRequested);

UCLASS(BlueprintType)
class PROJECTVEGA_API UProjectVegaBattleMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="BattleMenu")
    void SetAbilities(const TArray<UAbilityDataAsset*>& InAbilities);

    UPROPERTY(BlueprintAssignable, Category="BattleMenu")
    FOnBattleAbilitySelected OnAbilitySelected;

    UPROPERTY(BlueprintAssignable, Category="BattleMenu")
    FOnBattleEndTurnRequested OnEndTurnRequested;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleAbilityPicked(UAbilityDataAsset* Ability);
    void HandleEndTurn();

    TArray<TWeakObjectPtr<UAbilityDataAsset>> CachedAbilities;
    TSharedPtr<class SProjectVegaBattleMenu> BattleMenu;
};
