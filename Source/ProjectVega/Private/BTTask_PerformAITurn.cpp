#include "BTTask_PerformAITurn.h"
#include "AIController.h"
#include "AITurnComponent.h"

UBTTask_PerformAITurn::UBTTask_PerformAITurn()
{
    NodeName = TEXT("Perform AI Turn");
}

EBTNodeResult::Type UBTTask_PerformAITurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    UAITurnComponent* TurnComp = Pawn->FindComponentByClass<UAITurnComponent>();
    if (!TurnComp)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = nullptr;
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        if (!TargetActorKeyName.IsNone())
        {
            TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKeyName));
        }
    }

    const bool bResult = TurnComp->PerformAITurn(TargetActor);
    return bResult ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
