#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITurnComponent.generated.h"

class UAbilityDataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UAITurnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAITurnComponent();

    // Default ability this AI will use when taking its turn
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    UAbilityDataAsset* DefaultAbility = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    bool bUseAbilityLoadout = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    bool bRandomizeAbility = true;

    // If no target is passed, self will be targeted when true
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    bool bTargetSelfIfNoTarget = true;

    // Execute the AI action and end the turn
    UFUNCTION(BlueprintCallable, Category="AI")
    bool PerformAITurn(AActor* TargetOverride);
};
