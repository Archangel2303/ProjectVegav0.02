#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityCooldownComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UAbilityCooldownComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilityCooldownComponent();

    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsOnCooldown(FName AbilityName) const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void SetCooldown(FName AbilityName, int32 Turns);

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void TickTurn();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION()
    void HandleTurnAdvanced(int32 TurnNumber);

protected:
    // Remaining turns per ability
    UPROPERTY(VisibleAnywhere)
    TMap<FName, int32> RemainingCooldowns;
};
