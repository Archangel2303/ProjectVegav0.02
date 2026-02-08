#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityTypes.h"
#include "ActiveEffectComponent.generated.h"

USTRUCT(BlueprintType)
struct FActiveEffectInstance
{
    GENERATED_BODY()

    UPROPERTY()
    FEffectPayload Payload;

    UPROPERTY()
    int32 RemainingDuration = 0;

    UPROPERTY()
    int32 StackCount = 1;

    UPROPERTY()
    FGuid SourceId;

    UPROPERTY()
    FGuid InstanceId;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UActiveEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UActiveEffectComponent();

    UFUNCTION(BlueprintCallable)
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable)
    bool GetActiveEffectInstanceIdAt(int32 Index, FGuid& OutId) const;

    UFUNCTION(BlueprintCallable)
    FGuid AddActiveEffect(const FEffectPayload& Payload, const FGuid& SourceId, int32 InitialDuration);

    UFUNCTION(BlueprintCallable)
    bool RemoveActiveEffectById(const FGuid& InstanceId);

    UFUNCTION(BlueprintCallable)
    void TickTurn();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere)
    TArray<FActiveEffectInstance> ActiveEffects;

private:
    UFUNCTION()
    void HandleTurnAdvanced(int32 TurnNumber);
};
