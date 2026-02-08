#include "ProjectVegaPlayerController.h"
#include "TurnEndWidget.h"
#include "Blueprint/UserWidget.h"

AProjectVegaPlayerController::AProjectVegaPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AProjectVegaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (EndTurnWidgetClass)
    {
        EndTurnWidget = CreateWidget<UTurnEndWidget>(this, EndTurnWidgetClass);
        if (EndTurnWidget)
        {
            EndTurnWidget->AddToViewport();
        }
    }
}
