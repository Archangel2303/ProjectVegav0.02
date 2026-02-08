#include "Misc/AutomationTest.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "AugmentComponent.h"
#include "AttributeComponent.h"
#include "AugmentDataAsset.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAugmentEquipUnequipTest, "ProjectVega.Augment.EquipUnequip", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAugmentEquipUnequipTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.World() && Context.WorldType == EWorldType::Editor)
        {
            World = Context.World();
            break;
        }
    }
    if (!World)
    {
        AddError(TEXT("No editor world available for test."));
        return false;
    }

    // Spawn a transient actor to own components
    AActor* Actor = World->SpawnActor<AActor>();
    if (!Actor)
    {
        AddError(TEXT("Failed to spawn actor."));
        return false;
    }

    UAttributeComponent* Attr = NewObject<UAttributeComponent>(Actor);
    Attr->RegisterComponent();

    UAugmentComponent* AugComp = NewObject<UAugmentComponent>(Actor);
    AugComp->RegisterComponent();

    // Create a simple augment asset and equip
    UAugmentDataAsset* Aug = NewObject<UAugmentDataAsset>(GetTransientPackage(), NAME_None);
    Aug->Slot = EAugmentSlot::Core;
    Aug->EquipSoftcapPercent = 0.35f;

    bool bOk = AugComp->EquipAugment(Aug);
    TestTrue(TEXT("Equip returned true"), bOk);

    const UAugmentDataAsset* Equipped = AugComp->GetAugmentInSlot(EAugmentSlot::Core);
    TestTrue(TEXT("Augment present in slot"), Equipped == Aug);

    float Soft = Attr->GetAttribute(TEXT("SoftcapPercent"));
    TestEqual(TEXT("Softcap applied"), Soft, 0.35f);

    // Unequip
    bool bUne = AugComp->UnequipAugment(EAugmentSlot::Core);
    TestTrue(TEXT("Unequip returned true"), bUne);

    const UAugmentDataAsset* After = AugComp->GetAugmentInSlot(EAugmentSlot::Core);
    TestTrue(TEXT("Slot cleared after unequip"), After == nullptr);

    float Soft2 = Attr->GetAttribute(TEXT("SoftcapPercent"));
    TestEqual(TEXT("Softcap reverted"), Soft2, 0.f);

    // Cleanup
    Actor->Destroy();

    return true;
}

#endif
