#include "ProjectVegaAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectVegaAIController::AProjectVegaAIController()
{
    bAttachToPawn = true;

    // Default BT asset path: /Game/AI/BT_ProjectVega_Default
    static ConstructorHelpers::FObjectFinder<UBehaviorTree> DefaultBT(TEXT("/Game/AI/BT_ProjectVega_Default.BT_ProjectVega_Default"));
    if (DefaultBT.Succeeded())
    {
        DefaultBehaviorTree = DefaultBT.Object;
    }
}

void AProjectVegaAIController::BeginPlay()
{
    Super::BeginPlay();

    if (DefaultBehaviorTree)
    {
        RunBehaviorTree(DefaultBehaviorTree);
    }

    if (!TargetActorBlackboardKeyName.IsNone())
    {
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
            {
                BB->SetValueAsObject(TargetActorBlackboardKeyName, PlayerPawn);
            }
        }
    }
}
