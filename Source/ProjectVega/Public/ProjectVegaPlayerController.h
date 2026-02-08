#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectVegaPlayerController.generated.h"

class UTurnEndWidget;

UCLASS()
class PROJECTVEGA_API AProjectVegaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AProjectVegaPlayerController();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UTurnEndWidget> EndTurnWidgetClass;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UTurnEndWidget* EndTurnWidget = nullptr;
};
