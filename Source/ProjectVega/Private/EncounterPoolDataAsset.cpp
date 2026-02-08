#include "EncounterPoolDataAsset.h"
#include "EncounterDefinitionDataAsset.h"

UEncounterDefinitionDataAsset* UEncounterPoolDataAsset::PickEncounter(int32 Seed) const
{
    if (Encounters.Num() == 0)
    {
        return nullptr;
    }

    float TotalWeight = 0.f;
    for (const FWeightedEncounterEntry& Entry : Encounters)
    {
        if (Entry.Encounter && Entry.Weight > 0.f)
        {
            TotalWeight += Entry.Weight;
        }
    }

    if (TotalWeight <= KINDA_SMALL_NUMBER)
    {
        return Encounters[0].Encounter;
    }

    FRandomStream Stream(Seed != 0 ? Seed : FMath::Rand());
    const float Roll = Stream.FRandRange(0.f, TotalWeight);
    float Running = 0.f;

    for (const FWeightedEncounterEntry& Entry : Encounters)
    {
        if (!Entry.Encounter || Entry.Weight <= 0.f)
        {
            continue;
        }

        Running += Entry.Weight;
        if (Roll <= Running)
        {
            return Entry.Encounter;
        }
    }

    return Encounters[0].Encounter;
}
