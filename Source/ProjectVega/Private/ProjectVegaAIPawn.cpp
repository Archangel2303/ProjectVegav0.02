#include "ProjectVegaAIPawn.h"
#include "ProjectVegaAIController.h"
#include "AbilityExecutorComponent.h"
#include "AITurnComponent.h"

AProjectVegaAIPawn::AProjectVegaAIPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    AbilityExecutor = CreateDefaultSubobject<UAbilityExecutorComponent>(TEXT("AbilityExecutor"));
    AITurn = CreateDefaultSubobject<UAITurnComponent>(TEXT("AITurn"));

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AProjectVegaAIController::StaticClass();
}
