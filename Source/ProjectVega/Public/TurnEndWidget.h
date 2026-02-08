#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnEndWidget.generated.h"

class UButton;

UCLASS(BlueprintType)
class PROJECTVEGA_API UTurnEndWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Optional button bound in BP (named EndTurnButton by default)
    UPROPERTY(meta=(BindWidgetOptional))
    UButton* EndTurnButton = nullptr;

    UFUNCTION(BlueprintCallable, Category="Turn")
    void EndTurn();

protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleEndTurnClicked();
};
