#include "ProjectVegaPlayerCharacter.h"
#include "AbilityExecutorComponent.h"
#include "AugmentComponent.h"
#include "AttributeComponent.h"
#include "AbilityCooldownComponent.h"
#include "AugmentComponentExtensions.h"
#include "AbilityDataAsset.h"
#include "AugmentDataAsset.h"

AProjectVegaPlayerCharacter::AProjectVegaPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    AbilityExecutor = CreateDefaultSubobject<UAbilityExecutorComponent>(TEXT("AbilityExecutor"));
    AugmentComponent = CreateDefaultSubobject<UAugmentComponent>(TEXT("Augments"));
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
    CooldownComponent = CreateDefaultSubobject<UAbilityCooldownComponent>(TEXT("AbilityCooldowns"));
}

TArray<UAbilityDataAsset*> AProjectVegaPlayerCharacter::GetAvailableAbilities() const
{
    TArray<UAbilityDataAsset*> Out;

    if (AugmentComponent)
    {
        Out = UAugmentComponentExtensions::GetGrantedAbilities(AugmentComponent);
    }

    for (UAbilityDataAsset* Ability : DefaultAbilities)
    {
        if (Ability) Out.AddUnique(Ability);
    }

    return Out;
}

void AProjectVegaPlayerCharacter::ApplyLoadout(const TArray<UAugmentDataAsset*>& InAugments, const TArray<UAbilityDataAsset*>& InDefaultAbilities)
{
    DefaultAbilities = InDefaultAbilities;

    if (!AugmentComponent)
    {
        return;
    }

    AugmentComponent->ClearAllAugments();
    for (UAugmentDataAsset* Augment : InAugments)
    {
        if (Augment)
        {
            AugmentComponent->EquipAugment(Augment);
        }
    }
}
