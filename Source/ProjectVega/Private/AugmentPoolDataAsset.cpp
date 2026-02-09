#include "AugmentPoolDataAsset.h"
#include "AugmentDataAsset.h"

TArray<UAugmentDataAsset*> UAugmentPoolDataAsset::PickAugments(int32 Count, int32 Seed) const
{
    TArray<UAugmentDataAsset*> Out;

    if (Count <= 0 || Augments.Num() == 0)
    {
        return Out;
    }

    float TotalWeight = 0.f;
    for (const FAugmentPoolEntry& Entry : Augments)
    {
        if (Entry.Augment && Entry.Weight > 0.f)
        {
            TotalWeight += Entry.Weight;
        }
    }

    if (TotalWeight <= KINDA_SMALL_NUMBER)
    {
        for (const FAugmentPoolEntry& Entry : Augments)
        {
            if (Entry.Augment)
            {
                Out.AddUnique(Entry.Augment);
                if (Out.Num() >= Count)
                {
                    break;
                }
            }
        }
        return Out;
    }

    FRandomStream Stream(Seed != 0 ? Seed : FMath::Rand());
    TArray<FAugmentPoolEntry> Pool = Augments;

    for (int32 i = Pool.Num() - 1; i > 0; --i)
    {
        const int32 SwapIdx = Stream.RandRange(0, i);
        Pool.Swap(i, SwapIdx);
    }

    for (const FAugmentPoolEntry& Entry : Pool)
    {
        if (!Entry.Augment || Entry.Weight <= 0.f)
        {
            continue;
        }

        const float Roll = Stream.FRandRange(0.f, TotalWeight);
        float Running = 0.f;
        for (const FAugmentPoolEntry& Candidate : Pool)
        {
            if (!Candidate.Augment || Candidate.Weight <= 0.f)
            {
                continue;
            }

            Running += Candidate.Weight;
            if (Roll <= Running)
            {
                Out.AddUnique(Candidate.Augment);
                break;
            }
        }

        if (Out.Num() >= Count)
        {
            break;
        }
    }

    return Out;
}
