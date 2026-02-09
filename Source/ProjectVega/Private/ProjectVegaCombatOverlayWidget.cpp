#include "ProjectVegaCombatOverlayWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

class SProjectVegaCombatOverlay : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SProjectVegaCombatOverlay) {}
        SLATE_EVENT(FSimpleDelegate, OnToggleMap)
        SLATE_EVENT(FSimpleDelegate, OnResolveEncounter)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        OnToggleMap = InArgs._OnToggleMap;
        OnResolveEncounter = InArgs._OnResolveEncounter;

        ChildSlot
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Top)
            .Padding(FMargin(12.f))
            [
                SNew(SBorder)
                .Padding(FMargin(10.f))
                .BorderBackgroundColor(FLinearColor(0.05f, 0.08f, 0.12f, 0.85f))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
                    [
                        SNew(SButton)
                        .OnClicked(this, &SProjectVegaCombatOverlay::HandleToggleMap)
                        .ContentPadding(FMargin(10.f, 6.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("Map")))
                        ]
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .OnClicked(this, &SProjectVegaCombatOverlay::HandleResolveEncounter)
                        .ContentPadding(FMargin(10.f, 6.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("Resolve")))
                        ]
                    ]
                ]
            ]
        ];
    }

private:
    FReply HandleToggleMap()
    {
        if (OnToggleMap.IsBound())
        {
            OnToggleMap.Execute();
        }
        return FReply::Handled();
    }

    FReply HandleResolveEncounter()
    {
        if (OnResolveEncounter.IsBound())
        {
            OnResolveEncounter.Execute();
        }
        return FReply::Handled();
    }

    FSimpleDelegate OnToggleMap;
    FSimpleDelegate OnResolveEncounter;
};

TSharedRef<SWidget> UProjectVegaCombatOverlayWidget::RebuildWidget()
{
    return SNew(SProjectVegaCombatOverlay)
        .OnToggleMap(FSimpleDelegate::CreateUObject(this, &UProjectVegaCombatOverlayWidget::HandleToggleMap))
        .OnResolveEncounter(FSimpleDelegate::CreateUObject(this, &UProjectVegaCombatOverlayWidget::HandleResolveEncounter));
}

void UProjectVegaCombatOverlayWidget::HandleToggleMap()
{
    OnToggleMap.Broadcast();
}

void UProjectVegaCombatOverlayWidget::HandleResolveEncounter()
{
    OnResolveEncounter.Broadcast();
}
