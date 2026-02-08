#include "TurnEndWidget.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TurnManager.h"

void UTurnEndWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (EndTurnButton)
    {
        EndTurnButton->OnClicked.AddDynamic(this, &UTurnEndWidget::HandleEndTurnClicked);
    }
}

void UTurnEndWidget::HandleEndTurnClicked()
{
    EndTurn();
}

void UTurnEndWidget::EndTurn()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
            {
                TurnManager->EndTurn();
            }
        }
    }
}
