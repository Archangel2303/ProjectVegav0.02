#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectVegaPlayerCharacter.generated.h"

class UAbilityExecutorComponent;
class UAugmentComponent;
class UAttributeComponent;
class UAbilityCooldownComponent;
class UAbilityDataAsset;

UCLASS()
class PROJECTVEGA_API AProjectVegaPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AProjectVegaPlayerCharacter();

    UFUNCTION(BlueprintCallable, Category="Abilities")
    TArray<UAbilityDataAsset*> GetAvailableAbilities() const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    UAbilityExecutorComponent* GetAbilityExecutor() const { return AbilityExecutor; }

    UFUNCTION(BlueprintCallable, Category="Augments")
    UAugmentComponent* GetAugmentComponent() const { return AugmentComponent; }

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void ApplyLoadout(const TArray<UAugmentDataAsset*>& InAugments, const TArray<UAbilityDataAsset*>& InDefaultAbilities);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Abilities")
    TArray<UAbilityDataAsset*> DefaultAbilities;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    UAbilityExecutorComponent* AbilityExecutor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    UAugmentComponent* AugmentComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attributes")
    UAttributeComponent* AttributeComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    UAbilityCooldownComponent* CooldownComponent = nullptr;
};
