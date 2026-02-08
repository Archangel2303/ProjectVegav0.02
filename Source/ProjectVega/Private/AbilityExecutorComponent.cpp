#include "AbilityExecutorComponent.h"
#include "AbilitySystemSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UAbilityExecutorComponent::ExecuteAbilityByTargets(UAbilityDataAsset* Ability, const TArray<AActor*>& Targets, float Magnifier)
{
    FAbilityContext Ctx;
    Ctx.Caster = GetOwner();
    Ctx.Targets = Targets;
    Ctx.Magnifier = Magnifier;
    Ctx.Seed = FMath::Rand();

    ExecuteAbilityWithContext(Ability, Ctx);
}

void UAbilityExecutorComponent::ExecuteAbilityWithContext(UAbilityDataAsset* Ability, const FAbilityContext& Context)
{
    if (!GetWorld()) return;

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;

    UAbilitySystemSubsystem* Sub = GI->GetSubsystem<UAbilitySystemSubsystem>();
    if (Sub)
    {
        Sub->ExecuteAbility(Ability, Context);
    }
}
