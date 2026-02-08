#include "ProjectVegaRunStateSubsystem.h"
#include "FloorDefinitionDataAsset.h"
#include "EncounterPoolDataAsset.h"
#include "EncounterDefinitionDataAsset.h"
#include "ProjectVegaPlayerCharacter.h"
#include "AugmentDataAsset.h"
#include "AugmentComponent.h"
#include "AbilityDataAsset.h"
#include "Kismet/GameplayStatics.h"

void UProjectVegaRunStateSubsystem::GenerateFloorMap(UFloorDefinitionDataAsset* FloorDefinition, int32 Seed)
{
    CurrentFloor = FloorDefinition;
    FloorNodes.Reset();
    SelectedNodeId = INDEX_NONE;
    SelectedEncounter = nullptr;

    if (!CurrentFloor)
    {
        return;
    }

    FRandomStream Stream(Seed != 0 ? Seed : FMath::Rand());
    const int32 Rows = FMath::Max(2, CurrentFloor->Rows);
    const int32 Columns = FMath::Max(2, CurrentFloor->Columns);
    const int32 MinPerRow = FMath::Clamp(CurrentFloor->MinNodesPerRow, 1, Columns);
    const int32 MaxPerRow = FMath::Clamp(CurrentFloor->MaxNodesPerRow, MinPerRow, Columns);

    TArray<TArray<int32>> RowNodeIds;
    RowNodeIds.SetNum(Rows);

    auto AddNode = [&](int32 Row, int32 Column) -> int32
    {
        FFloorMapNode Node;
        Node.NodeId = FloorNodes.Num();
        Node.Row = Row;
        Node.Column = Column;
        Node.MapPosition = FVector2D(
            Columns > 1 ? static_cast<float>(Column) / static_cast<float>(Columns - 1) : 0.5f,
            Rows > 1 ? 1.f - static_cast<float>(Row) / static_cast<float>(Rows - 1) : 0.5f
        );
        FloorNodes.Add(Node);
        return Node.NodeId;
    };

    // Generate rows (except boss row)
    for (int32 Row = 0; Row < Rows - 1; ++Row)
    {
        const int32 NodeCount = Stream.RandRange(MinPerRow, MaxPerRow);
        TArray<int32> ColumnsAvailable;
        for (int32 Col = 0; Col < Columns; ++Col)
        {
            ColumnsAvailable.Add(Col);
        }

        for (int32 i = ColumnsAvailable.Num() - 1; i > 0; --i)
        {
            const int32 SwapIdx = Stream.RandRange(0, i);
            ColumnsAvailable.Swap(i, SwapIdx);
        }

        ColumnsAvailable.SetNum(NodeCount);
        ColumnsAvailable.Sort();

        for (int32 Col : ColumnsAvailable)
        {
            const int32 NodeId = AddNode(Row, Col);
            RowNodeIds[Row].Add(NodeId);
        }
    }

    // Boss row (single node)
    const int32 BossColumn = Columns / 2;
    const int32 BossNodeId = AddNode(Rows - 1, BossColumn);
    RowNodeIds[Rows - 1].Add(BossNodeId);

    // Assign node types, encounters, and difficulty
    int32 MiniBossAssigned = 0;
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 NodeId : RowNodeIds[Row])
        {
            FFloorMapNode& Node = FloorNodes[NodeId];

            if (Row == Rows - 1)
            {
                Node.NodeType = EFloorNodeType::Boss;
                Node.Difficulty = EEncounterDifficulty::Hard;
                Node.Encounter = PickEncounterFromPool(CurrentFloor->BossPool, Stream);
                continue;
            }

            if (Row == Rows - 2 && MiniBossAssigned < CurrentFloor->NumMiniBossNodes)
            {
                Node.NodeType = EFloorNodeType::MiniBoss;
                Node.Difficulty = EEncounterDifficulty::Hard;
                Node.Encounter = PickEncounterFromPool(CurrentFloor->MiniBossPool, Stream);
                ++MiniBossAssigned;
                continue;
            }

            const float CombatW = FMath::Max(0.f, CurrentFloor->CombatNodeWeight);
            const float EliteW = FMath::Max(0.f, CurrentFloor->EliteNodeWeight);
            const float RestW = FMath::Max(0.f, CurrentFloor->RestNodeWeight);
            const float ShopW = FMath::Max(0.f, CurrentFloor->ShopNodeWeight);
            const float EventW = FMath::Max(0.f, CurrentFloor->EventNodeWeight);
            const float Total = CombatW + EliteW + RestW + ShopW + EventW;
            float Roll = Stream.FRandRange(0.f, Total > KINDA_SMALL_NUMBER ? Total : 1.f);

            if (Roll <= CombatW)
            {
                Node.NodeType = EFloorNodeType::Combat;
                Node.Difficulty = RollNormalDifficulty(Stream);
            }
            else if (Roll <= CombatW + EliteW)
            {
                Node.NodeType = EFloorNodeType::Elite;
                Node.Difficulty = EEncounterDifficulty::Hard;
            }
            else if (Roll <= CombatW + EliteW + RestW)
            {
                Node.NodeType = EFloorNodeType::Rest;
            }
            else if (Roll <= CombatW + EliteW + RestW + ShopW)
            {
                Node.NodeType = EFloorNodeType::Shop;
            }
            else
            {
                Node.NodeType = EFloorNodeType::Event;
            }

            if (Node.NodeType == EFloorNodeType::Combat)
            {
                switch (Node.Difficulty)
                {
                    case EEncounterDifficulty::Easy:
                        Node.Encounter = PickEncounterFromPool(CurrentFloor->EasyPool, Stream);
                        break;
                    case EEncounterDifficulty::Medium:
                        Node.Encounter = PickEncounterFromPool(CurrentFloor->MediumPool, Stream);
                        break;
                    case EEncounterDifficulty::Hard:
                        Node.Encounter = PickEncounterFromPool(CurrentFloor->HardPool, Stream);
                        break;
                }
            }
            else if (Node.NodeType == EFloorNodeType::Elite)
            {
                Node.Encounter = PickEncounterFromPool(CurrentFloor->MiniBossPool ? CurrentFloor->MiniBossPool : CurrentFloor->HardPool, Stream);
            }
        }
    }

    // Create links between rows
    for (int32 Row = 0; Row < Rows - 1; ++Row)
    {
        for (int32 NodeId : RowNodeIds[Row])
        {
            FFloorMapNode& Node = FloorNodes[NodeId];
            const TArray<int32>& NextRow = RowNodeIds[Row + 1];
            if (NextRow.Num() == 0)
            {
                continue;
            }

            TArray<int32> CandidateIds;
            for (int32 NextId : NextRow)
            {
                const int32 ColDiff = FMath::Abs(FloorNodes[NextId].Column - Node.Column);
                if (ColDiff <= 1)
                {
                    CandidateIds.Add(NextId);
                }
            }

            if (CandidateIds.Num() == 0)
            {
                int32 BestDiff = INT_MAX;
                for (int32 NextId : NextRow)
                {
                    const int32 ColDiff = FMath::Abs(FloorNodes[NextId].Column - Node.Column);
                    if (ColDiff < BestDiff)
                    {
                        BestDiff = ColDiff;
                        CandidateIds.Reset();
                        CandidateIds.Add(NextId);
                    }
                    else if (ColDiff == BestDiff)
                    {
                        CandidateIds.Add(NextId);
                    }
                }
            }

            for (int32 i = CandidateIds.Num() - 1; i > 0; --i)
            {
                const int32 SwapIdx = Stream.RandRange(0, i);
                CandidateIds.Swap(i, SwapIdx);
            }

            const int32 LinkCount = (CandidateIds.Num() > 1 && Stream.FRand() < 0.35f) ? 2 : 1;
            for (int32 i = 0; i < CandidateIds.Num() && i < LinkCount; ++i)
            {
                Node.LinkedNodeIds.AddUnique(CandidateIds[i]);
            }
        }
    }
}

bool UProjectVegaRunStateSubsystem::SelectNode(int32 NodeId)
{
    for (const FFloorMapNode& Node : FloorNodes)
    {
        if (Node.NodeId == NodeId)
        {
            SelectedNodeId = NodeId;
            SelectedEncounter = Node.Encounter;
            return true;
        }
    }
    return false;
}

void UProjectVegaRunStateSubsystem::StartSelectedEncounter()
{
    if (!CurrentFloor || !SelectedEncounter)
    {
        return;
    }

    if (CurrentFloor->EncounterMapName.IsNone())
    {
        return;
    }

    UGameplayStatics::OpenLevel(GetWorld(), CurrentFloor->EncounterMapName);
}

void UProjectVegaRunStateSubsystem::CapturePlayerLoadout(AProjectVegaPlayerCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    PlayerLoadout.DefaultAbilities = Player->DefaultAbilities;

    if (UAugmentComponent* Augments = Player->GetAugmentComponent())
    {
        PlayerLoadout.EquippedAugments = Augments->GetEquippedAugments();
    }
}

void UProjectVegaRunStateSubsystem::ApplyLoadout(AProjectVegaPlayerCharacter* Player) const
{
    if (!Player)
    {
        return;
    }

    Player->ApplyLoadout(PlayerLoadout.EquippedAugments, PlayerLoadout.DefaultAbilities);
}

UEncounterDefinitionDataAsset* UProjectVegaRunStateSubsystem::PickEncounterFromPool(UEncounterPoolDataAsset* Pool, FRandomStream& Stream) const
{
    if (!Pool)
    {
        return nullptr;
    }

    const int32 Seed = Stream.RandRange(1, INT32_MAX);
    return Pool->PickEncounter(Seed);
}

EEncounterDifficulty UProjectVegaRunStateSubsystem::RollNormalDifficulty(FRandomStream& Stream) const
{
    const float EasyWeight = FMath::Max(0.f, CurrentFloor ? CurrentFloor->EasyWeight : 0.f);
    const float MediumWeight = FMath::Max(0.f, CurrentFloor ? CurrentFloor->MediumWeight : 0.f);
    const float HardWeight = FMath::Max(0.f, CurrentFloor ? CurrentFloor->HardWeight : 0.f);
    const float Total = EasyWeight + MediumWeight + HardWeight;

    if (Total <= KINDA_SMALL_NUMBER)
    {
        return EEncounterDifficulty::Easy;
    }

    const float Roll = Stream.FRandRange(0.f, Total);
    if (Roll <= EasyWeight)
    {
        return EEncounterDifficulty::Easy;
    }
    if (Roll <= EasyWeight + MediumWeight)
    {
        return EEncounterDifficulty::Medium;
    }
    return EEncounterDifficulty::Hard;
}
