#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ProjectVegaAIController.generated.h"

class UBehaviorTree;

UCLASS()
class PROJECTVEGA_API AProjectVegaAIController : public AAIController
{
    GENERATED_BODY()

public:
    AProjectVegaAIController();

    // Behavior Tree asset to run on BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    UBehaviorTree* DefaultBehaviorTree = nullptr;

    // Blackboard key to auto-set to the player pawn on BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    FName TargetActorBlackboardKeyName = TEXT("TargetActor");

protected:
    virtual void BeginPlay() override;
};
