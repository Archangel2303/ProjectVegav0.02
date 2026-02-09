#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EEffectType : uint8
{
    Damage,
    Armor,
    DOT,
    Buff,
    Heal,
    Unknown
};

UENUM(BlueprintType)
enum class EAbilityCategory : uint8
{
    Offensive,
    Defensive,
    Utility,
    Hack
};

UENUM(BlueprintType)
enum class EAbilitySpecialType : uint8
{
    None,
    Overdrive,
    Herculean
};

UENUM(BlueprintType)
enum class EAbilityTargetingMode : uint8
{
    Single,
    Splash,
    AllEnemies,
    AllAllies,
    Everyone,
    RandomEnemies,
    RandomAllies,
    LowestHealthEnemy,
    LowestHealthAlly,
    HighestHealthEnemy,
    HighestHealthAlly
};

USTRUCT(BlueprintType)
struct FDamageEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Amount = 0.f;
};

USTRUCT(BlueprintType)
struct FArmorEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Amount = 0.f;
};

USTRUCT(BlueprintType)
struct FEffectPayload
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEffectType EffectType = EEffectType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Duration = 0; // in turns; 0 = instant

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStacks = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDamageEffectData DamageParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FArmorEffectData ArmorParams;
};

USTRUCT(BlueprintType)
struct FAbilityContext
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Caster = nullptr;

    UPROPERTY()
    TArray<AActor*> Targets;

    UPROPERTY()
    float Magnifier = 1.0f;

    // Deterministic seed for RNG in handlers
    UPROPERTY()
    int32 Seed = 0;
};

USTRUCT(BlueprintType)
struct FEffectResult
{
    GENERATED_BODY()

    // Map attribute name to delta value
    UPROPERTY()
    TMap<FName, float> AttributeDeltas;

    UPROPERTY()
    FGameplayTagContainer GrantedTags;
};
