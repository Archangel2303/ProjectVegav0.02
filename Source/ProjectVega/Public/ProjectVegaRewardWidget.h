#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjectVegaRewardWidget.generated.h"

class UAugmentDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardNanitesAccepted, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardAugmentChosen, UAugmentDataAsset*, Augment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewardContinue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewardVialOpened);

UCLASS(BlueprintType)
class PROJECTVEGA_API UProjectVegaRewardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Rewards")
    void InitializeReward(int32 NaniteAmount, const TArray<UAugmentDataAsset*>& AugmentOptions);

    UPROPERTY(BlueprintAssignable, Category="Rewards")
    FOnRewardNanitesAccepted OnNanitesAccepted;

    UPROPERTY(BlueprintAssignable, Category="Rewards")
    FOnRewardAugmentChosen OnAugmentChosen;

    UPROPERTY(BlueprintAssignable, Category="Rewards")
    FOnRewardContinue OnContinue;

    UPROPERTY(BlueprintAssignable, Category="Rewards")
    FOnRewardVialOpened OnVialOpened;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleAcceptNanites();
    void HandleShowVial();
    void HandleChooseAugment(int32 Index);
    void HandleContinue();

    int32 CurrentNanites = 0;
    TArray<UAugmentDataAsset*> CurrentOptions;
    bool bShowingAugments = false;
    bool bNanitesClaimed = false;
    bool bAugmentClaimed = false;
    bool bVialOpened = false;
    TSharedPtr<class SProjectVegaRewardPrompt> RewardSlate;
};
