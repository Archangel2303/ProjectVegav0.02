#include "ProjectVegaDeathWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

class SProjectVegaDeathPrompt : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SProjectVegaDeathPrompt) {}
        SLATE_EVENT(FSimpleDelegate, OnRetry)
        SLATE_EVENT(FSimpleDelegate, OnQuit)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        OnRetry = InArgs._OnRetry;
        OnQuit = InArgs._OnQuit;

        ChildSlot
        [
            SNew(SBorder)
            .Padding(FMargin(24.f))
            .BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.85f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(FMargin(0.f, 0.f, 0.f, 12.f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("You died")))
                    .ColorAndOpacity(FLinearColor(1.f, 0.3f, 0.3f, 1.f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(FMargin(0.f, 0.f, 0.f, 8.f))
                [
                    SNew(SButton)
                    .OnClicked(this, &SProjectVegaDeathPrompt::HandleRetry)
                    .ContentPadding(FMargin(12.f, 6.f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Play again")))
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SButton)
                    .OnClicked(this, &SProjectVegaDeathPrompt::HandleQuit)
                    .ContentPadding(FMargin(12.f, 6.f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quit")))
                    ]
                ]
            ]
        ];
    }

private:
    FReply HandleRetry()
    {
        if (OnRetry.IsBound())
        {
            OnRetry.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleQuit()
    {
        if (OnQuit.IsBound())
        {
            OnQuit.Execute();
        }
        return FReply::Handled();
    }

    FSimpleDelegate OnRetry;
    FSimpleDelegate OnQuit;
};

TSharedRef<SWidget> UProjectVegaDeathWidget::RebuildWidget()
{
    return SNew(SProjectVegaDeathPrompt)
        .OnRetry(FSimpleDelegate::CreateUObject(this, &UProjectVegaDeathWidget::HandleRetry))
        .OnQuit(FSimpleDelegate::CreateUObject(this, &UProjectVegaDeathWidget::HandleQuit));
}

void UProjectVegaDeathWidget::HandleRetry()
{
    OnRetry.Broadcast();
}

void UProjectVegaDeathWidget::HandleQuit()
{
    OnQuit.Broadcast();
}
