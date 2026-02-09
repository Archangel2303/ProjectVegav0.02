#include "ProjectVegaRewardWidget.h"
#include "AugmentDataAsset.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

class SProjectVegaRewardPrompt : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SProjectVegaRewardPrompt) {}
        SLATE_EVENT(FSimpleDelegate, OnAcceptNanites)
        SLATE_EVENT(FSimpleDelegate, OnShowVial)
        SLATE_EVENT(FSimpleDelegate, OnChooseAugment0)
        SLATE_EVENT(FSimpleDelegate, OnChooseAugment1)
        SLATE_EVENT(FSimpleDelegate, OnChooseAugment2)
        SLATE_EVENT(FSimpleDelegate, OnContinue)
        SLATE_ARGUMENT(int32, NaniteAmount)
        SLATE_ARGUMENT(TArray<FString>, AugmentNames)
        SLATE_ARGUMENT(bool, bShowingAugments)
        SLATE_ARGUMENT(bool, bNanitesClaimed)
        SLATE_ARGUMENT(bool, bAugmentClaimed)
        SLATE_ARGUMENT(bool, bVialOpened)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        OnAcceptNanites = InArgs._OnAcceptNanites;
        OnShowVial = InArgs._OnShowVial;
        OnChooseAugment0 = InArgs._OnChooseAugment0;
        OnChooseAugment1 = InArgs._OnChooseAugment1;
        OnChooseAugment2 = InArgs._OnChooseAugment2;
        OnContinue = InArgs._OnContinue;
        NaniteAmount = InArgs._NaniteAmount;
        AugmentNames = InArgs._AugmentNames;
        bShowingAugments = InArgs._bShowingAugments;
        bNanitesClaimed = InArgs._bNanitesClaimed;
        bAugmentClaimed = InArgs._bAugmentClaimed;
        bVialOpened = InArgs._bVialOpened;

        ChildSlot
        [
            SNew(SBox)
            .WidthOverride(320.f)
            [
                SNew(SBorder)
                .Padding(FMargin(14.f))
                .BorderBackgroundColor(FLinearColor(0.02f, 0.04f, 0.06f, 0.9f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 0.f, 0.f, 8.f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Encounter Reward")))
                        .ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.f, 1.f))
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 0.f, 0.f, 8.f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(FString::Printf(TEXT("Nanites: %d"), NaniteAmount)))
                        .ColorAndOpacity(FLinearColor(0.9f, 0.85f, 0.3f, 1.f))
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
                    [
                        SNew(SButton)
                        .OnClicked(this, &SProjectVegaRewardPrompt::HandleAcceptNanites)
                        .ContentPadding(FMargin(10.f, 6.f))
                        .IsEnabled(!bNanitesClaimed)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(bNanitesClaimed ? TEXT("Nanites Claimed") : TEXT("Take Nanites")))
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SButton)
                        .OnClicked(this, &SProjectVegaRewardPrompt::HandleShowVial)
                        .ContentPadding(FMargin(10.f, 6.f))
                        .IsEnabled(!bAugmentClaimed)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(bAugmentClaimed ? TEXT("Augment Claimed") : (bVialOpened ? TEXT("Vial Opened") : TEXT("Nanite Vial"))))
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 12.f, 0.f, 0.f))
                    [
                        BuildAugmentPanel()
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 12.f, 0.f, 0.f))
                    [
                        SNew(SButton)
                        .OnClicked(this, &SProjectVegaRewardPrompt::HandleContinue)
                        .ContentPadding(FMargin(10.f, 6.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("Continue")))
                        ]
                    ]
                ]
            ]
        ];
    }

    void SetShowingAugments(bool bInShowing)
    {
        bShowingAugments = bInShowing;
        Invalidate(EInvalidateWidgetReason::Layout);
    }

    void SetNanitesClaimed(bool bClaimed)
    {
        bNanitesClaimed = bClaimed;
        Invalidate(EInvalidateWidgetReason::Layout);
    }

    void SetAugmentClaimed(bool bClaimed)
    {
        bAugmentClaimed = bClaimed;
        Invalidate(EInvalidateWidgetReason::Layout);
    }

    void SetVialOpened(bool bOpened)
    {
        bVialOpened = bOpened;
        Invalidate(EInvalidateWidgetReason::Layout);
    }

private:
    TSharedRef<SWidget> BuildAugmentPanel()
    {
        if (!bShowingAugments)
        {
            return SNew(STextBlock)
                .Text(FText::GetEmpty());
        }

        const FString A0 = AugmentNames.IsValidIndex(0) ? AugmentNames[0] : TEXT("(Empty)");
        const FString A1 = AugmentNames.IsValidIndex(1) ? AugmentNames[1] : TEXT("(Empty)");
        const FString A2 = AugmentNames.IsValidIndex(2) ? AugmentNames[2] : TEXT("(Empty)");

        return SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Choose one augment")))
                .ColorAndOpacity(FLinearColor(0.7f, 0.9f, 0.8f, 1.f))
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
            [
                SNew(SButton)
                .OnClicked(this, &SProjectVegaRewardPrompt::HandleChooseAugment0)
                .ContentPadding(FMargin(12.f, 6.f))
                .IsEnabled(!bAugmentClaimed)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(A0))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
            [
                SNew(SButton)
                .OnClicked(this, &SProjectVegaRewardPrompt::HandleChooseAugment1)
                .ContentPadding(FMargin(12.f, 6.f))
                .IsEnabled(!bAugmentClaimed)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(A1))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
                .OnClicked(this, &SProjectVegaRewardPrompt::HandleChooseAugment2)
                .ContentPadding(FMargin(12.f, 6.f))
                .IsEnabled(!bAugmentClaimed)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(A2))
                ]
            ];
    }

    FReply HandleAcceptNanites()
    {
        if (OnAcceptNanites.IsBound())
        {
            OnAcceptNanites.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleShowVial()
    {
        if (OnShowVial.IsBound())
        {
            OnShowVial.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleChooseAugment0()
    {
        if (OnChooseAugment0.IsBound())
        {
            OnChooseAugment0.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleChooseAugment1()
    {
        if (OnChooseAugment1.IsBound())
        {
            OnChooseAugment1.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleChooseAugment2()
    {
        if (OnChooseAugment2.IsBound())
        {
            OnChooseAugment2.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleContinue()
    {
        if (OnContinue.IsBound())
        {
            OnContinue.Execute();
        }
        return FReply::Handled();
    }

    FSimpleDelegate OnAcceptNanites;
    FSimpleDelegate OnShowVial;
    FSimpleDelegate OnChooseAugment0;
    FSimpleDelegate OnChooseAugment1;
    FSimpleDelegate OnChooseAugment2;
    FSimpleDelegate OnContinue;
    int32 NaniteAmount = 0;
    TArray<FString> AugmentNames;
    bool bShowingAugments = false;
    bool bNanitesClaimed = false;
    bool bAugmentClaimed = false;
    bool bVialOpened = false;
};

void UProjectVegaRewardWidget::InitializeReward(int32 NaniteAmount, const TArray<UAugmentDataAsset*>& AugmentOptions)
{
    CurrentNanites = NaniteAmount;
    CurrentOptions = AugmentOptions;
    bNanitesClaimed = false;
    bAugmentClaimed = false;
    bVialOpened = false;
    bShowingAugments = false;
}

TSharedRef<SWidget> UProjectVegaRewardWidget::RebuildWidget()
{
    TArray<FString> AugmentNames;
    for (UAugmentDataAsset* Augment : CurrentOptions)
    {
        AugmentNames.Add(Augment ? Augment->GetName() : TEXT("(Empty)"));
    }

    return SAssignNew(RewardSlate, SProjectVegaRewardPrompt)
        .NaniteAmount(CurrentNanites)
        .AugmentNames(AugmentNames)
        .bShowingAugments(bShowingAugments)
        .bNanitesClaimed(bNanitesClaimed)
        .bAugmentClaimed(bAugmentClaimed)
        .bVialOpened(bVialOpened)
        .OnAcceptNanites(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleAcceptNanites))
        .OnShowVial(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleShowVial))
        .OnChooseAugment0(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleChooseAugment, 0))
        .OnChooseAugment1(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleChooseAugment, 1))
        .OnChooseAugment2(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleChooseAugment, 2))
        .OnContinue(FSimpleDelegate::CreateUObject(this, &UProjectVegaRewardWidget::HandleContinue));
}

void UProjectVegaRewardWidget::HandleAcceptNanites()
{
    if (bNanitesClaimed)
    {
        return;
    }
    bNanitesClaimed = true;
    if (RewardSlate.IsValid())
    {
        RewardSlate->SetNanitesClaimed(true);
    }
    OnNanitesAccepted.Broadcast(CurrentNanites);
}

void UProjectVegaRewardWidget::HandleShowVial()
{
    if (bVialOpened)
    {
        return;
    }
    bVialOpened = true;
    bShowingAugments = true;
    if (RewardSlate.IsValid())
    {
        RewardSlate->SetShowingAugments(true);
        RewardSlate->SetVialOpened(true);
    }
    OnVialOpened.Broadcast();
}

void UProjectVegaRewardWidget::HandleChooseAugment(int32 Index)
{
    if (bAugmentClaimed)
    {
        return;
    }
    if (CurrentOptions.IsValidIndex(Index))
    {
        bAugmentClaimed = true;
        if (RewardSlate.IsValid())
        {
            RewardSlate->SetAugmentClaimed(true);
        }
        OnAugmentChosen.Broadcast(CurrentOptions[Index]);
    }
}

void UProjectVegaRewardWidget::HandleContinue()
{
    OnContinue.Broadcast();
}
