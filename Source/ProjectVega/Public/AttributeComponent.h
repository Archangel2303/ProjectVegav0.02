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
        float GetEffectiveMaxHealth() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Attributes")
    TMap<FName, float> Attributes;
};
