#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AugmentTypes.h"
#include "AugmentDataAsset.h"
#include "AugmentComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTVEGA_API UAugmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAugmentComponent();

    UFUNCTION(BlueprintCallable, Category="Augments")
    bool EquipAugment(UAugmentDataAsset* Augment);

    UFUNCTION(BlueprintCallable, Category="Augments")
    bool UnequipAugment(EAugmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Augments")
    const UAugmentDataAsset* GetAugmentInSlot(EAugmentSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category="Augments")
    TArray<UAugmentDataAsset*> GetEquippedAugments() const;

protected:
    virtual void BeginPlay() override;

    // Map slot to currently equipped augment
    UPROPERTY(VisibleAnywhere)
    TMap<EAugmentSlot, UAugmentDataAsset*> Equipped;

    // Map slot to instance IDs of passive effects granted when this augment was equipped
    TMap<EAugmentSlot, TArray<FGuid>> EquippedPassiveEffectIds;
};
