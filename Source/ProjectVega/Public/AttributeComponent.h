#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributeComponent();

    UFUNCTION(BlueprintCallable, Category="Attributes")
    float GetAttribute(FName AttributeName) const;

    UFUNCTION(BlueprintCallable, Category="Attributes")
    float ApplyAttributeDelta(FName AttributeName, float Delta);

    UFUNCTION(BlueprintCallable, Category="Attributes")
    void SetAttribute(FName AttributeName, float Value);

        UFUNCTION(BlueprintCallable, Category="Attributes")
        float GetEffectiveMaxHealth() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleTurnAdvanced(int32 TurnNumber);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    void SyncDefaultsToMap(bool bForce);

    UPROPERTY(EditAnywhere, Category="Attributes|Defaults")
    float DefaultHealth = 100.f;

    UPROPERTY(EditAnywhere, Category="Attributes|Defaults")
    float DefaultBaseMaxHealth = 100.f;

    UPROPERTY(EditAnywhere, Category="Attributes|Defaults")
    float DefaultArmor = 0.f;

    UPROPERTY(EditAnywhere, Category="Attributes|Defaults")
    float DefaultSoftcapPercent = 0.f;

    UPROPERTY(EditAnywhere, Category="Attributes")
    TMap<FName, float> Attributes;
};
