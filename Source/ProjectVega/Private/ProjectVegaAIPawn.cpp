#include "ProjectVegaAIPawn.h"
#include "ProjectVegaAIController.h"
#include "AbilityExecutorComponent.h"
#include "AITurnComponent.h"
#include "AttributeComponent.h"

AProjectVegaAIPawn::AProjectVegaAIPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    AbilityExecutor = CreateDefaultSubobject<UAbilityExecutorComponent>(TEXT("AbilityExecutor"));
    AITurn = CreateDefaultSubobject<UAITurnComponent>(TEXT("AITurn"));
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AProjectVegaAIController::StaticClass();
}

void AProjectVegaAIPawn::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (AbilityLoadout.Num() > 8)
    {
        AbilityLoadout.SetNum(8);
    }
}
