#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaCombatOverlayWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatToggleMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatResolveEncounter);

UCLASS(BlueprintType)
class PROJECTVEGA_API UProjectVegaCombatOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category="Combat")
    FOnCombatToggleMap OnToggleMap;

    UPROPERTY(BlueprintAssignable, Category="Combat")
    FOnCombatResolveEncounter OnResolveEncounter;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleToggleMap();
    void HandleResolveEncounter();
};
