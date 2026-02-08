#include "TurnManager.h"

void UTurnManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentTurn = 0;
}

void UTurnManager::AdvanceTurn()
{
    ++CurrentTurn;
    OnTurnAdvanced.Broadcast(CurrentTurn);
}

void UTurnManager::EndTurn()
{
    AdvanceTurn();
}
