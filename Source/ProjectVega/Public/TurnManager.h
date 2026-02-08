#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TurnManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnAdvanced, int32, TurnNumber);

UCLASS()
class PROJECTVEGA_API UTurnManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable)
    void AdvanceTurn();

    // Call when a player ends their turn (UI) or when AI finishes its action
    UFUNCTION(BlueprintCallable)
    void EndTurn();

    UPROPERTY(BlueprintAssignable)
    FOnTurnAdvanced OnTurnAdvanced;

    UFUNCTION(BlueprintCallable)
    int32 GetCurrentTurn() const { return CurrentTurn; }

private:
    int32 CurrentTurn = 0;
};
