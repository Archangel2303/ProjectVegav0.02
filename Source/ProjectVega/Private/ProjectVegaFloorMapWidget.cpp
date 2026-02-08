#include "ProjectVegaFloorMapWidget.h"
#include "ProjectVegaRunStateSubsystem.h"
#include "EncounterDefinitionDataAsset.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

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
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SAssignNew(NodeList, SVerticalBox)
                ]
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
        if (!NodeList.IsValid())
        {
            return;
        }

        NodeList->ClearChildren();

        for (const FFloorMapNode& Node : Nodes)
        {
            NodeList->AddSlot()
            .AutoHeight()
            .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
            [
                SNew(SButton)
                .OnClicked(this, &SProjectVegaFloorMap::HandleNodeClicked, Node.NodeId)
                .ContentPadding(FMargin(10.f, 6.f))
                [
                    SNew(STextBlock)
                    .Text(BuildNodeText(Node))
                    .ColorAndOpacity(BuildNodeColor(Node))
                ]
            ];
        }
    }

    FText BuildNodeText(const FFloorMapNode& Node) const
    {
        FString TypeLabel = TEXT("Normal");
        if (Node.NodeType == EFloorNodeType::MiniBoss)
        {
            TypeLabel = TEXT("Mini Boss");
        }
        else if (Node.NodeType == EFloorNodeType::Boss)
        {
            TypeLabel = TEXT("Boss");
        }

        FString DiffLabel = TEXT("Easy");
        if (Node.Difficulty == EEncounterDifficulty::Medium)
        {
            DiffLabel = TEXT("Medium");
        }
        else if (Node.Difficulty == EEncounterDifficulty::Hard)
        {
            DiffLabel = TEXT("Hard");
        }

        FString Name = Node.Encounter ? Node.Encounter->EncounterName.ToString() : TEXT("Unknown");
        return FText::FromString(FString::Printf(TEXT("Node %d - %s (%s) - %s"), Node.NodeId, *TypeLabel, *DiffLabel, *Name));
    }

    FSlateColor BuildNodeColor(const FFloorMapNode& Node) const
    {
        switch (Node.Difficulty)
        {
            case EEncounterDifficulty::Easy:
                return FSlateColor(FLinearColor(0.4f, 0.9f, 0.5f, 1.f));
            case EEncounterDifficulty::Medium:
                return FSlateColor(FLinearColor(0.9f, 0.7f, 0.2f, 1.f));
            case EEncounterDifficulty::Hard:
                return FSlateColor(FLinearColor(0.95f, 0.3f, 0.3f, 1.f));
            default:
                return FSlateColor(FLinearColor::White);
        }
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
    TSharedPtr<SVerticalBox> NodeList;
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
