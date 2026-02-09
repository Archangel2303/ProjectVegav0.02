#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloorMapTypes.h"
#include "ProjectVegaFloorMapWidget.generated.h"

class UProjectVegaRunStateSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorNodeChosen, int32, NodeId);

UCLASS(BlueprintType)
class PROJECTVEGA_API UProjectVegaFloorMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="FloorMap")
    void SetRunState(UProjectVegaRunStateSubsystem* InRunState);

    UFUNCTION(BlueprintCallable, Category="FloorMap")
    void RefreshNodes();

    UFUNCTION(BlueprintCallable, Category="FloorMap")
    void SetAutoStartEncounter(bool bInAutoStart) { bAutoStartEncounter = bInAutoStart; }

    UFUNCTION(BlueprintCallable, Category="FloorMap")
    void SetUseBackdrop(bool bInUseBackdrop) { bUseBackdrop = bInUseBackdrop; }

    UPROPERTY(BlueprintAssignable, Category="FloorMap")
    FOnFloorNodeChosen OnNodeChosen;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void HandleNodeClicked(int32 NodeId);

    TWeakObjectPtr<UProjectVegaRunStateSubsystem> RunState;
    TArray<FFloorMapNode> CachedNodes;
    TSet<int32> SelectableNodeIds;
    TSharedPtr<class SProjectVegaFloorMap> FloorMapSlate;
    bool bAutoStartEncounter = true;
    bool bUseBackdrop = false;
};
