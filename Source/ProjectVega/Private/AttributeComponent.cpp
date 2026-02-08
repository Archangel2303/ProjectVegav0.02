#include "AttributeComponent.h"
#include "GameFramework/Actor.h"

UAttributeComponent::UAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
    Super::BeginPlay();

    // Ensure some sensible defaults (can be overridden in editor)
    if (!Attributes.Contains(TEXT("Health")))
    {
        Attributes.Add(TEXT("Health"), 100.f);
    }
    if (!Attributes.Contains(TEXT("BaseMaxHealth")))
    {
        Attributes.Add(TEXT("BaseMaxHealth"), 100.f);
    }
    if (!Attributes.Contains(TEXT("SoftcapPercent")))
    {
        Attributes.Add(TEXT("SoftcapPercent"), 0.f);
    }
    if (!Attributes.Contains(TEXT("Focus")))
    {
        Attributes.Add(TEXT("Focus"), 0.f);
    }
}

float UAttributeComponent::GetAttribute(FName AttributeName) const
{
    if (AttributeName == TEXT("MaxHealth"))
    {
        return GetEffectiveMaxHealth();
    }

    const float* Val = Attributes.Find(AttributeName);
    return Val ? *Val : 0.f;
}

float UAttributeComponent::ApplyAttributeDelta(FName AttributeName, float Delta)
{
    // Special handling for health to respect effective max and not auto-restore
    if (AttributeName == TEXT("Health"))
    {
        float& Curr = Attributes.FindOrAdd(TEXT("Health"), 0.f);
        Curr += Delta;
        float MaxEff = GetEffectiveMaxHealth();
        Curr = FMath::Clamp(Curr, 0.f, MaxEff);
        return Curr;
    }

    // SoftcapPercent stored directly
    if (AttributeName == TEXT("SoftcapPercent"))
    {
        float& Val = Attributes.FindOrAdd(TEXT("SoftcapPercent"), 0.f);
        Val += Delta;
        Val = FMath::Clamp(Val, 0.f, 1.f);
        // If increasing the softcap reduces the effective max below current Health,
        // immediately clamp current Health down to the new effective max.
        float MaxEff = GetEffectiveMaxHealth();
        float& Curr = Attributes.FindOrAdd(TEXT("Health"), 0.f);
        if (Curr > MaxEff)
        {
            Curr = MaxEff;
        }
        return Val;
    }

    float& Val = Attributes.FindOrAdd(AttributeName, 0.f);
    Val += Delta;
    return Val;
}

float UAttributeComponent::GetEffectiveMaxHealth() const
{
    const float* BasePtr = Attributes.Find(TEXT("BaseMaxHealth"));
    float Base = BasePtr ? *BasePtr : 0.f;
    const float* SoftPtr = Attributes.Find(TEXT("SoftcapPercent"));
    float Soft = SoftPtr ? *SoftPtr : 0.f;
    // Effective max is Base * (1 - SoftcapPercent)
    return Base * FMath::Clamp(1.f - Soft, 0.f, 1.f);
}
