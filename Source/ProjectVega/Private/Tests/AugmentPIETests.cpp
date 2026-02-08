#include "Misc/AutomationTest.h"
#if WITH_EDITOR
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "AugmentComponent.h"
#include "AttributeComponent.h"
#include "ActiveEffectComponent.h"
#include "AugmentDataAsset.h"
#include "AbilityDataAsset.h"
#include "AbilityTypes.h"

class FAugmentPIERunCommand : public IAutomationLatentCommand
{
public:
    explicit FAugmentPIERunCommand(FAutomationTestBase* InTest)
        : Test(InTest)
    {
    }

    virtual bool Update() override
    {
        if (bDone)
        {
            return true;
        }

        if (!GEditor || !GEditor->PlayWorld)
        {
            return false; // wait for PIE world
        }

        UWorld* World = GEditor->PlayWorld;
        if (!World)
        {
            Test->AddError(TEXT("No PIE world available."));
            bDone = true;
            return true;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        if (!Actor)
        {
            Test->AddError(TEXT("Failed to spawn actor in PIE."));
            bDone = true;
            return true;
        }

        UAttributeComponent* Attr = NewObject<UAttributeComponent>(Actor);
        Attr->RegisterComponent();

        UActiveEffectComponent* Active = NewObject<UActiveEffectComponent>(Actor);
        Active->RegisterComponent();

        UAugmentComponent* AugComp = NewObject<UAugmentComponent>(Actor);
        AugComp->RegisterComponent();

        UAbilityDataAsset* Passive = NewObject<UAbilityDataAsset>(GetTransientPackage(), NAME_None);
        Passive->AbilityName = TEXT("TestPassive");
        Passive->bTargetSelf = true;
        FEffectPayload Payload;
        Payload.EffectType = EEffectType::Damage;
        Payload.Duration = 2;
        Payload.DamageParams.Amount = 0.f;
        Passive->Effects.Add(Payload);

        UAugmentDataAsset* Aug = NewObject<UAugmentDataAsset>(GetTransientPackage(), NAME_None);
        Aug->Slot = EAugmentSlot::Core;
        Aug->ExtraPassiveAbility = Passive;

        bool bEquip = AugComp->EquipAugment(Aug);
        Test->TestTrue(TEXT("Equip returned true"), bEquip);
        Test->TestEqual(TEXT("Active effect created"), Active->GetActiveEffectCount(), 1);

        FGuid FirstId;
        if (Active->GetActiveEffectInstanceIdAt(0, FirstId))
        {
            Test->TestTrue(TEXT("InstanceId valid"), FirstId.IsValid());
        }

        bool bUne = AugComp->UnequipAugment(EAugmentSlot::Core);
        Test->TestTrue(TEXT("Unequip returned true"), bUne);
        Test->TestEqual(TEXT("Active effects removed"), Active->GetActiveEffectCount(), 0);

        Actor->Destroy();

        bDone = true;
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    bool bDone = false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAugmentPIEPassiveEffectTest, "ProjectVega.Augment.PIEPassiveEffects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAugmentPIEPassiveEffectTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FAugmentPIERunCommand(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}

#endif
