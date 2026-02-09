#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaDeathWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathRetry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathQuit);

UCLASS(BlueprintType)
class PROJECTVEGA_API UProjectVegaDeathWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category="Death")
    FOnDeathRetry OnRetry;

    UPROPERTY(BlueprintAssignable, Category="Death")
    FOnDeathQuit OnQuit;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleRetry();
    void HandleQuit();
};
