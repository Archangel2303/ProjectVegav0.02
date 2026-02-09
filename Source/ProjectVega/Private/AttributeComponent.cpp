#include "AttributeComponent.h"
#include "GameFramework/Actor.h"
#include "TurnManager.h"

UAttributeComponent::UAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
    Super::BeginPlay();

    SyncDefaultsToMap(false);

    if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTurnManager* TurnManager = GI->GetSubsystem<UTurnManager>())
        {
            TurnManager->OnTurnAdvanced.AddDynamic(this, &UAttributeComponent::HandleTurnAdvanced);
        }
    }
}

void UAttributeComponent::HandleTurnAdvanced(int32 TurnNumber)
{
    Attributes.FindOrAdd(TEXT("Armor"), 0.f) = 0.f;
}

#if WITH_EDITOR
void UAttributeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    SyncDefaultsToMap(true);
}
#endif

void UAttributeComponent::SyncDefaultsToMap(bool bForce)
{
    if (bForce || !Attributes.Contains(TEXT("Health")))
    {
        Attributes.Add(TEXT("Health"), DefaultHealth);
    }
    if (bForce || !Attributes.Contains(TEXT("BaseMaxHealth")))
    {
        Attributes.Add(TEXT("BaseMaxHealth"), DefaultBaseMaxHealth);
    }
    if (bForce || !Attributes.Contains(TEXT("Armor")))
    {
        Attributes.Add(TEXT("Armor"), DefaultArmor);
    }
    if (bForce || !Attributes.Contains(TEXT("SoftcapPercent")))
    {
        Attributes.Add(TEXT("SoftcapPercent"), DefaultSoftcapPercent);
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
        if (Delta < 0.f)
        {
            float& Armor = Attributes.FindOrAdd(TEXT("Armor"), 0.f);
            const float Incoming = -Delta;
            const float Absorbed = FMath::Min(Armor, Incoming);
            Armor -= Absorbed;
            Curr -= (Incoming - Absorbed);
        }
        else
        {
            Curr += Delta;
        }
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
    if (AttributeName == TEXT("Armor"))
    {
        Val = FMath::Max(0.f, Val);
    }
    return Val;
}

void UAttributeComponent::SetAttribute(FName AttributeName, float Value)
{
    if (AttributeName == TEXT("Health"))
    {
        const float MaxEff = GetEffectiveMaxHealth();
        Attributes.FindOrAdd(TEXT("Health"), 0.f) = FMath::Clamp(Value, 0.f, MaxEff);
        return;
    }

    if (AttributeName == TEXT("SoftcapPercent"))
    {
        Attributes.FindOrAdd(TEXT("SoftcapPercent"), 0.f) = FMath::Clamp(Value, 0.f, 1.f);
        const float MaxEff = GetEffectiveMaxHealth();
        float& Curr = Attributes.FindOrAdd(TEXT("Health"), 0.f);
        if (Curr > MaxEff)
        {
            Curr = MaxEff;
        }
        return;
    }

    Attributes.FindOrAdd(AttributeName, 0.f) = Value;
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
