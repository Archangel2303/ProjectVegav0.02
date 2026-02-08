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

    const int32 TotalNodes = FMath::Max(0, CurrentFloor->NumNormalNodes) +
        FMath::Max(0, CurrentFloor->NumMiniBossNodes) +
        FMath::Max(0, CurrentFloor->NumBossNodes);

    FRandomStream Stream(Seed != 0 ? Seed : FMath::Rand());
    int32 NodeId = 0;

    for (int32 i = 0; i < CurrentFloor->NumNormalNodes; ++i)
    {
        FFloorMapNode Node;
        Node.NodeId = NodeId++;
        Node.NodeType = EFloorNodeType::Normal;
        Node.Difficulty = RollNormalDifficulty(Stream);

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

        FloorNodes.Add(Node);
    }

    for (int32 i = 0; i < CurrentFloor->NumMiniBossNodes; ++i)
    {
        FFloorMapNode Node;
        Node.NodeId = NodeId++;
        Node.NodeType = EFloorNodeType::MiniBoss;
        Node.Difficulty = EEncounterDifficulty::Hard;
        Node.Encounter = PickEncounterFromPool(CurrentFloor->MiniBossPool, Stream);
        FloorNodes.Add(Node);
    }

    for (int32 i = 0; i < CurrentFloor->NumBossNodes; ++i)
    {
        FFloorMapNode Node;
        Node.NodeId = NodeId++;
        Node.NodeType = EFloorNodeType::Boss;
        Node.Difficulty = EEncounterDifficulty::Hard;
        Node.Encounter = PickEncounterFromPool(CurrentFloor->BossPool, Stream);
        FloorNodes.Add(Node);
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
