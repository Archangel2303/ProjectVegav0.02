#include "ProjectVegaFloorMapWidget.h"
#include "ProjectVegaRunStateSubsystem.h"
#include "EncounterDefinitionDataAsset.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"

DECLARE_DELEGATE_OneParam(FOnFloorNodeClickedNative, int32);

class SProjectVegaFloorMap : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SProjectVegaFloorMap) {}
        SLATE_EVENT(FOnFloorNodeClickedNative, OnNodeClicked)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        OnNodeClicked = InArgs._OnNodeClicked;

        ChildSlot
        [
            SNew(SBorder)
            .Padding(FMargin(12.f))
            .BorderBackgroundColor(FLinearColor(0.05f, 0.07f, 0.1f, 0.9f))
            [
                SAssignNew(Canvas, SConstraintCanvas)
            ]
        ];
    }

    void SetNodes(const TArray<FFloorMapNode>& InNodes)
    {
        Nodes = InNodes;
        RefreshList();
    }

private:
    void RefreshList()
    {
        if (!Canvas.IsValid())
        {
            return;
        }

        Canvas->ClearChildren();

        const FVector2D NodeSize(28.f, 28.f);

        for (const FFloorMapNode& Node : Nodes)
        {
            Canvas->AddSlot()
            .Anchors(FAnchors(Node.MapPosition.X, Node.MapPosition.Y))
            .Alignment(FVector2D(0.5f, 0.5f))
            .Offset(FMargin(0.f, 0.f, NodeSize.X, NodeSize.Y))
            [
                SNew(SButton)
                .OnClicked(this, &SProjectVegaFloorMap::HandleNodeClicked, Node.NodeId)
                .ContentPadding(FMargin(4.f, 2.f))
                [
                    SNew(STextBlock)
                    .Text(BuildNodeText(Node))
                    .Justification(ETextJustify::Center)
                    .ColorAndOpacity(BuildNodeColor(Node))
                ]
            ];
        }
    }

    FText BuildNodeText(const FFloorMapNode& Node) const
    {
        switch (Node.NodeType)
        {
            case EFloorNodeType::Combat:
                return FText::FromString(TEXT("C"));
            case EFloorNodeType::Elite:
                return FText::FromString(TEXT("E"));
            case EFloorNodeType::Rest:
                return FText::FromString(TEXT("R"));
            case EFloorNodeType::Shop:
                return FText::FromString(TEXT("S"));
            case EFloorNodeType::Event:
                return FText::FromString(TEXT("?"));
            case EFloorNodeType::MiniBoss:
                return FText::FromString(TEXT("M"));
            case EFloorNodeType::Boss:
                return FText::FromString(TEXT("B"));
            default:
                return FText::FromString(TEXT("C"));
        }
    }

    FSlateColor BuildNodeColor(const FFloorMapNode& Node) const
    {
        switch (Node.NodeType)
        {
            case EFloorNodeType::Combat:
                return FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f));
            case EFloorNodeType::Elite:
                return FSlateColor(FLinearColor(1.f, 0.55f, 0.2f, 1.f));
            case EFloorNodeType::Rest:
                return FSlateColor(FLinearColor(0.4f, 0.9f, 0.5f, 1.f));
            case EFloorNodeType::Shop:
                return FSlateColor(FLinearColor(0.35f, 0.7f, 1.f, 1.f));
            case EFloorNodeType::Event:
                return FSlateColor(FLinearColor(0.9f, 0.8f, 0.35f, 1.f));
            case EFloorNodeType::MiniBoss:
                return FSlateColor(FLinearColor(0.95f, 0.3f, 0.3f, 1.f));
            case EFloorNodeType::Boss:
                return FSlateColor(FLinearColor(0.75f, 0.2f, 0.2f, 1.f));
            default:
                return FSlateColor(FLinearColor::White);
        }
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        const FVector2D Size = AllottedGeometry.GetLocalSize();

        for (const FFloorMapNode& Node : Nodes)
        {
            const FVector2D Start = FVector2D(Node.MapPosition.X * Size.X, Node.MapPosition.Y * Size.Y);
            for (int32 LinkedId : Node.LinkedNodeIds)
            {
                if (!Nodes.IsValidIndex(LinkedId))
                {
                    continue;
                }
                const FFloorMapNode& Target = Nodes[LinkedId];
                const FVector2D End = FVector2D(Target.MapPosition.X * Size.X, Target.MapPosition.Y * Size.Y);
                TArray<FVector2D> Points;
                Points.Add(Start);
                Points.Add(End);
                FSlateDrawElement::MakeLines(
                    OutDrawElements,
                    LayerId,
                    AllottedGeometry.ToPaintGeometry(),
                    Points,
                    ESlateDrawEffect::None,
                    FLinearColor(0.35f, 0.4f, 0.45f, 0.7f),
                    true,
                    1.2f
                );
            }
        }

        return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 1, InWidgetStyle, bParentEnabled);
    }

    FReply HandleNodeClicked(int32 NodeId)
    {
        if (OnNodeClicked.IsBound())
        {
            OnNodeClicked.Execute(NodeId);
        }
        return FReply::Handled();
    }

private:
    TArray<FFloorMapNode> Nodes;
    TSharedPtr<SConstraintCanvas> Canvas;
    FOnFloorNodeClickedNative OnNodeClicked;
};

void UProjectVegaFloorMapWidget::SetRunState(UProjectVegaRunStateSubsystem* InRunState)
{
    RunState = InRunState;
    RefreshNodes();
}

void UProjectVegaFloorMapWidget::RefreshNodes()
{
    CachedNodes.Reset();

    if (RunState.IsValid())
    {
        CachedNodes = RunState->GetFloorNodes();
    }

    if (FloorMapSlate.IsValid())
    {
        FloorMapSlate->SetNodes(CachedNodes);
    }
}

TSharedRef<SWidget> UProjectVegaFloorMapWidget::RebuildWidget()
{
    FloorMapSlate = SNew(SProjectVegaFloorMap)
        .OnNodeClicked(FOnFloorNodeClickedNative::CreateUObject(this, &UProjectVegaFloorMapWidget::HandleNodeClicked));

    FloorMapSlate->SetNodes(CachedNodes);
    return FloorMapSlate.ToSharedRef();
}

void UProjectVegaFloorMapWidget::HandleNodeClicked(int32 NodeId)
{
    if (RunState.IsValid())
    {
        RunState->SelectNode(NodeId);
        if (bAutoStartEncounter)
        {
            RunState->StartSelectedEncounter();
        }
    }

    OnNodeChosen.Broadcast(NodeId);
}
