#include "ProjectVegaRunStateSubsystem.h"
#include "FloorDefinitionDataAsset.h"
#include "EncounterPoolDataAsset.h"
#include "EncounterDefinitionDataAsset.h"
#include "ProjectVegaPlayerCharacter.h"
#include "AugmentDataAsset.h"
#include "AugmentComponent.h"
#include "AbilityDataAsset.h"
#include "AugmentPoolDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Misc/PackageName.h"

void UProjectVegaRunStateSubsystem::GenerateFloorMap(UFloorDefinitionDataAsset* FloorDefinition, int32 Seed)
{
    if (bHasGeneratedMap && !bPendingRegenerate && CurrentFloor == FloorDefinition)
    {
        return;
    }

    CurrentFloor = FloorDefinition;
    CachedFloorMapName = CurrentFloor ? CurrentFloor->FloorMapName : NAME_None;
    CachedEncounterMapName = CurrentFloor ? CurrentFloor->EncounterMapName : NAME_None;
    FloorNodes.Reset();
    SelectedNodeId = INDEX_NONE;
    SelectedEncounter = nullptr;
    CurrentNodeId = INDEX_NONE;
    LastSeed = Seed;
    bHasGeneratedMap = false;

    if (bPendingRegenerate)
    {
        bPendingRegenerate = false;
    }

    if (!CurrentFloor)
    {
        return;
    }

    FRandomStream Stream(Seed != 0 ? Seed : FMath::Rand());
    LastSeed = Seed;
    const int32 Rows = FMath::Max(2, CurrentFloor->Rows);
    const int32 Columns = FMath::Max(2, CurrentFloor->Columns);
    const int32 MinPerRow = FMath::Clamp(CurrentFloor->MinNodesPerRow, 1, Columns);
    const int32 MaxPerRow = FMath::Clamp(CurrentFloor->MaxNodesPerRow, MinPerRow, Columns);
    const int32 EntryMin = FMath::Clamp(CurrentFloor->EntryMinNodesPerRow, 1, Columns);
    const int32 EntryMax = FMath::Clamp(CurrentFloor->EntryMaxNodesPerRow, EntryMin, Columns);

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
        const int32 NodeCount = (Row == 0) ? Stream.RandRange(EntryMin, EntryMax) : Stream.RandRange(MinPerRow, MaxPerRow);
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

            const float CombatW = FMath::Max(0.f, CurrentFloor->CombatNodeWeight);
            const float EliteW = FMath::Max(0.f, CurrentFloor->EliteNodeWeight);
            const float RestW = FMath::Max(0.f, CurrentFloor->RestNodeWeight);
            const float ShopW = FMath::Max(0.f, CurrentFloor->ShopNodeWeight);
            const float EventW = FMath::Max(0.f, CurrentFloor->EventNodeWeight);
            const float Total = CombatW + EliteW + RestW + ShopW + EventW;
            float Roll = Stream.FRandRange(0.f, Total > KINDA_SMALL_NUMBER ? Total : 1.f);
            if (Total <= KINDA_SMALL_NUMBER)
            {
                UE_LOG(LogTemp, Warning, TEXT("Floor generation: all node weights are zero. Defaulting to Event nodes."));
            }

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

    // Mix miniboss nodes into the floor (excluding boss row)
    if (CurrentFloor->NumMiniBossNodes > 0)
    {
        TArray<int32> Eligible;
        for (const FFloorMapNode& Node : FloorNodes)
        {
            if (Node.Row < Rows - 1)
            {
                Eligible.Add(Node.NodeId);
            }
        }

        for (int32 i = Eligible.Num() - 1; i > 0; --i)
        {
            const int32 SwapIdx = Stream.RandRange(0, i);
            Eligible.Swap(i, SwapIdx);
        }

        const int32 ToAssign = FMath::Min(CurrentFloor->NumMiniBossNodes, Eligible.Num());
        for (int32 i = 0; i < ToAssign; ++i)
        {
            const int32 NodeId = Eligible[i];
            if (!FloorNodes.IsValidIndex(NodeId))
            {
                continue;
            }
            FFloorMapNode& Node = FloorNodes[NodeId];
            Node.NodeType = EFloorNodeType::MiniBoss;
            Node.Difficulty = EEncounterDifficulty::Hard;
            Node.Encounter = PickEncounterFromPool(CurrentFloor->MiniBossPool, Stream);
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

    if (bLogFloorGeneration)
    {
        DumpFloorMapToLog();
    }

    bHasGeneratedMap = true;
}

bool UProjectVegaRunStateSubsystem::IsNodeSelectable(int32 NodeId) const
{
    auto GetEntryRow = [](const TArray<FFloorMapNode>& Nodes) -> int32
    {
        int32 MinRow = MAX_int32;
        for (const FFloorMapNode& Entry : Nodes)
        {
            MinRow = FMath::Min(MinRow, Entry.Row);
        }
        return MinRow == MAX_int32 ? INDEX_NONE : MinRow;
    };

    if (!FloorNodes.IsValidIndex(NodeId))
    {
        return false;
    }

    const FFloorMapNode& Node = FloorNodes[NodeId];
    if (Node.bCleared)
    {
        return false;
    }

    const int32 EntryRow = GetEntryRow(FloorNodes);

    if (CurrentNodeId == INDEX_NONE || !FloorNodes.IsValidIndex(CurrentNodeId))
    {
        return EntryRow != INDEX_NONE && Node.Row == EntryRow;
    }

    const FFloorMapNode& Current = FloorNodes[CurrentNodeId];
    if (Current.LinkedNodeIds.Num() == 0)
    {
        return EntryRow != INDEX_NONE && Node.Row == EntryRow;
    }
    return Current.LinkedNodeIds.Contains(NodeId);
}

TArray<int32> UProjectVegaRunStateSubsystem::GetSelectableNodeIds() const
{
    TArray<int32> Out;
    int32 EntryRow = MAX_int32;
    for (const FFloorMapNode& Node : FloorNodes)
    {
        EntryRow = FMath::Min(EntryRow, Node.Row);
    }

    const bool bHasEntryRow = EntryRow != MAX_int32;
    if (CurrentNodeId == INDEX_NONE || !FloorNodes.IsValidIndex(CurrentNodeId))
    {
        if (bHasEntryRow)
        {
            for (const FFloorMapNode& Node : FloorNodes)
            {
                if (Node.Row == EntryRow && !Node.bCleared)
                {
                    Out.Add(Node.NodeId);
                }
            }
        }
        return Out;
    }

    const FFloorMapNode& Current = FloorNodes[CurrentNodeId];
    if (Current.LinkedNodeIds.Num() == 0)
    {
        if (bHasEntryRow)
        {
            for (const FFloorMapNode& Node : FloorNodes)
            {
                if (Node.Row == EntryRow && !Node.bCleared)
                {
                    Out.Add(Node.NodeId);
                }
            }
        }
        return Out;
    }
    for (int32 LinkedId : Current.LinkedNodeIds)
    {
        if (FloorNodes.IsValidIndex(LinkedId) && !FloorNodes[LinkedId].bCleared)
        {
            Out.Add(LinkedId);
        }
    }
    return Out;
}

void UProjectVegaRunStateSubsystem::DumpFloorMapToLog() const
{
    int32 CombatCount = 0;
    int32 EliteCount = 0;
    int32 RestCount = 0;
    int32 ShopCount = 0;
    int32 EventCount = 0;
    int32 MiniBossCount = 0;
    int32 BossCount = 0;
    int32 LinkCount = 0;

    for (const FFloorMapNode& Node : FloorNodes)
    {
        LinkCount += Node.LinkedNodeIds.Num();
        switch (Node.NodeType)
        {
            case EFloorNodeType::Combat: CombatCount++; break;
            case EFloorNodeType::Elite: EliteCount++; break;
            case EFloorNodeType::Rest: RestCount++; break;
            case EFloorNodeType::Shop: ShopCount++; break;
            case EFloorNodeType::Event: EventCount++; break;
            case EFloorNodeType::MiniBoss: MiniBossCount++; break;
            case EFloorNodeType::Boss: BossCount++; break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("FloorMap: nodes=%d links=%d (C:%d E:%d R:%d S:%d ?: %d M:%d B:%d)"),
        FloorNodes.Num(), LinkCount, CombatCount, EliteCount, RestCount, ShopCount, EventCount, MiniBossCount, BossCount);

    for (const FFloorMapNode& Node : FloorNodes)
    {
        FString Links;
        for (int32 Id : Node.LinkedNodeIds)
        {
            Links += FString::Printf(TEXT("%d "), Id);
        }
        UE_LOG(LogTemp, Verbose, TEXT("Node %d row=%d col=%d type=%d links=[%s]"),
            Node.NodeId, Node.Row, Node.Column, static_cast<int32>(Node.NodeType), *Links);
    }
}

bool UProjectVegaRunStateSubsystem::SelectNode(int32 NodeId)
{
    for (const FFloorMapNode& Node : FloorNodes)
    {
        if (Node.NodeId == NodeId)
        {
            if (!IsNodeSelectable(NodeId))
            {
                return false;
            }
            SelectedNodeId = NodeId;
            SelectedEncounter = Node.Encounter;
            CachedFloorMapName = CurrentFloor ? CurrentFloor->FloorMapName : CachedFloorMapName;
            CachedEncounterMapName = CurrentFloor ? CurrentFloor->EncounterMapName : CachedEncounterMapName;
            if (CachedFloorMapName.IsNone())
            {
                CachedFloorMapName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
            }
            if (FloorNodes.IsValidIndex(NodeId))
            {
                FloorNodes[NodeId].bVisited = true;
            }
            return true;
        }
    }
    return false;
}

void UProjectVegaRunStateSubsystem::StartSelectedEncounter()
{
    if (!CurrentFloor)
    {
        return;
    }

    CachedFloorMapName = CurrentFloor->FloorMapName;
    CachedEncounterMapName = CurrentFloor->EncounterMapName;
    if (CachedFloorMapName.IsNone())
    {
        CachedFloorMapName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
    }

    if (CurrentFloor->EncounterMapName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSelectedEncounter: EncounterMapName not set on floor definition."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("Encounter map name not set."));
        }
        return;
    }

    if (!SelectedEncounter)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSelectedEncounter: No encounter assigned, opening map for testing."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("No encounter assigned; opening test map."));
        }
    }

    FString MapToOpen = CurrentFloor->EncounterMapName.ToString();
    if (!FPackageName::DoesPackageExist(MapToOpen))
    {
        const FString RootPath = FString::Printf(TEXT("/Game/%s"), *MapToOpen);
        if (FPackageName::DoesPackageExist(RootPath))
        {
            MapToOpen = RootPath;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("StartSelectedEncounter: Map package not found: %s"), *MapToOpen);
            if (GEngine)
            {
                const FString Message = FString::Printf(TEXT("Map not found: %s (use /Game/Maps/YourMap)"), *MapToOpen);
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Message);
            }
            return;
        }
    }

    UGameplayStatics::OpenLevel(GetWorld(), FName(*MapToOpen));
}

void UProjectVegaRunStateSubsystem::MarkEncounterResolved(bool bPlayerWon)
{
    if (bPlayerWon && FloorNodes.IsValidIndex(SelectedNodeId))
    {
        FloorNodes[SelectedNodeId].bCleared = true;
        CurrentNodeId = SelectedNodeId;

        if (FloorNodes[SelectedNodeId].NodeType == EFloorNodeType::Boss)
        {
            bFloorCleared = true;
            OnFloorCleared.Broadcast();
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Floor 1 cleared"));
            }
        }
    }

    SelectedNodeId = INDEX_NONE;
    SelectedEncounter = nullptr;
    ReturnToFloorMap(false);
}

void UProjectVegaRunStateSubsystem::ReturnToFloorMap(bool bRegenerate)
{
    FName TargetMap = GetFloorMapName();
    if (TargetMap.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("ReturnToFloorMap: Floor map name not set."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("ReturnToFloorMap: no floor map name"));
        }
        return;
    }

    if (bRegenerate)
    {
        bPendingRegenerate = true;
    }

    UGameplayStatics::OpenLevel(GetWorld(), TargetMap);
}

void UProjectVegaRunStateSubsystem::ResetRun(bool bRegenerate)
{
    SelectedNodeId = INDEX_NONE;
    SelectedEncounter = nullptr;
    CurrentNodeId = INDEX_NONE;
    bFloorCleared = false;
    Nanites = 0;
    AugmentInventory.Reset();

    if (bRegenerate)
    {
        bPendingRegenerate = true;
    }
}

void UProjectVegaRunStateSubsystem::AddNanites(int32 Amount)
{
    Nanites = FMath::Max(0, Nanites + Amount);
}

void UProjectVegaRunStateSubsystem::AddAugmentToInventory(UAugmentDataAsset* Augment)
{
    if (Augment)
    {
        AugmentInventory.AddUnique(Augment);
    }
}

TArray<UAugmentDataAsset*> UProjectVegaRunStateSubsystem::RollRewardAugments(int32 Count)
{
    if (!CurrentFloor || !CurrentFloor->FloorAugmentPool)
    {
        return TArray<UAugmentDataAsset*>();
    }

    return CurrentFloor->FloorAugmentPool->PickAugments(Count);
}

int32 UProjectVegaRunStateSubsystem::RollRewardNanites()
{
    if (!CurrentFloor)
    {
        return 0;
    }

    const int32 Min = FMath::Min(CurrentFloor->RewardNanitesMin, CurrentFloor->RewardNanitesMax);
    const int32 Max = FMath::Max(CurrentFloor->RewardNanitesMin, CurrentFloor->RewardNanitesMax);
    return FMath::RandRange(Min, Max);
}

void UProjectVegaRunStateSubsystem::AdvanceToNextFloor()
{
    bFloorCleared = false;
    CurrentFloorNumber = FMath::Max(1, CurrentFloorNumber + 1);
    bPendingRegenerate = true;
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

FName UProjectVegaRunStateSubsystem::GetFloorMapName() const
{
    if (CurrentFloor && !CurrentFloor->FloorMapName.IsNone())
    {
        return CurrentFloor->FloorMapName;
    }
    return CachedFloorMapName;
}

FName UProjectVegaRunStateSubsystem::GetEncounterMapName() const
{
    if (CurrentFloor && !CurrentFloor->EncounterMapName.IsNone())
    {
        return CurrentFloor->EncounterMapName;
    }
    return CachedEncounterMapName;
}

void UProjectVegaRunStateSubsystem::CacheFloorMapName(FName MapName)
{
    if (!MapName.IsNone())
    {
        CachedFloorMapName = MapName;
    }
}

void UProjectVegaRunStateSubsystem::CacheEncounterMapName(FName MapName)
{
    if (!MapName.IsNone())
    {
        CachedEncounterMapName = MapName;
    }
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
