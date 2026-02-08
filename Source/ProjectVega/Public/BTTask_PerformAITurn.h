#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTTask_PerformAITurn.generated.h"

UCLASS()
class PROJECTVEGA_API UBTTask_PerformAITurn : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PerformAITurn();

    // Optional blackboard key name that holds a target actor
    UPROPERTY(EditAnywhere, Category="AI")
    FName TargetActorKeyName = NAME_None;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
