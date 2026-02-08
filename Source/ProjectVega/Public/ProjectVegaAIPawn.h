#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProjectVegaAIPawn.generated.h"

class UAbilityExecutorComponent;
class UAITurnComponent;

UCLASS()
class PROJECTVEGA_API AProjectVegaAIPawn : public APawn
{
    GENERATED_BODY()

public:
    AProjectVegaAIPawn();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    UAbilityExecutorComponent* AbilityExecutor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    UAITurnComponent* AITurn = nullptr;
};
