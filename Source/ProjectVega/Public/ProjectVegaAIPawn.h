#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProjectVegaAIPawn.generated.h"

class UAbilityExecutorComponent;
class UAITurnComponent;
class UAttributeComponent;
class UAbilityDataAsset;

UCLASS()
class PROJECTVEGA_API AProjectVegaAIPawn : public APawn
{
    GENERATED_BODY()

public:
    AProjectVegaAIPawn();

    UFUNCTION(BlueprintCallable, Category="AI")
    const TArray<UAbilityDataAsset*>& GetAbilityLoadout() const { return AbilityLoadout; }

protected:
    virtual void PostInitializeComponents() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    UAbilityExecutorComponent* AbilityExecutor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    UAITurnComponent* AITurn = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attributes")
    UAttributeComponent* AttributeComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI", meta=(ClampMin="0", ClampMax="8"))
    TArray<UAbilityDataAsset*> AbilityLoadout;
};
