#include "ProjectVegaBattleMenuWidget.h"
#include "AbilityDataAsset.h"
#include "AbilityTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

enum class EBattleMenuMode : uint8
{
	Commands,
	Abilities,
	Items
};

DECLARE_DELEGATE_OneParam(FOnAbilityPickedNative, UAbilityDataAsset*);
DECLARE_DELEGATE(FOnEndTurnNative);

class SProjectVegaBattleMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectVegaBattleMenu) {}
		SLATE_EVENT(FOnAbilityPickedNative, OnAbilityPicked)
		SLATE_EVENT(FOnEndTurnNative, OnEndTurn)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		OnAbilityPicked = InArgs._OnAbilityPicked;
		OnEndTurn = InArgs._OnEndTurn;
		MenuMode = EBattleMenuMode::Commands;

		ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(12.f))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.08f, 0.12f, 0.85f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					BuildCommandColumn()
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(12.f, 0.f, 0.f, 0.f))
				[
					SAssignNew(RightPanel, SBorder)
					.Padding(FMargin(8.f))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.85f))
					[
						BuildRightPanel()
					]
				]
			]
		];
	}

	void SetAbilities(const TArray<TWeakObjectPtr<UAbilityDataAsset>>& InAbilities)
	{
		Abilities = InAbilities;
		HoveredAbilityIndex = INDEX_NONE;
		RefreshAbilityList();
	}

private:
	TSharedRef<SWidget> BuildCommandColumn()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 6.f))
			[
				BuildCommandButton(TEXT("Abilities"), 0, &SProjectVegaBattleMenu::HandleAbilities)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 6.f))
			[
				BuildCommandButton(TEXT("Items"), 1, &SProjectVegaBattleMenu::HandleItems)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				BuildCommandButton(TEXT("End Turn"), 2, &SProjectVegaBattleMenu::HandleEndTurn)
			];
	}

	TSharedRef<SWidget> BuildCommandButton(const FString& Label, int32 Index, FReply(SProjectVegaBattleMenu::*Handler)())
	{
		return SNew(SButton)
			.OnClicked(this, Handler)
			.OnHovered(FSimpleDelegate::CreateSP(this, &SProjectVegaBattleMenu::HandleCommandHovered, Index))
			.OnUnhovered(FSimpleDelegate::CreateSP(this, &SProjectVegaBattleMenu::HandleCommandUnhovered, Index))
			.ContentPadding(FMargin(12.f, 6.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.ColorAndOpacity(this, &SProjectVegaBattleMenu::GetCommandColorForIndex, Index)
				.ShadowOffset(this, &SProjectVegaBattleMenu::GetCommandShadowOffsetForIndex, Index)
				.ShadowColorAndOpacity(this, &SProjectVegaBattleMenu::GetCommandShadowColorForIndex, Index)
			];
	}

	TSharedRef<SWidget> BuildRightPanel()
	{
		return SAssignNew(AbilityListBox, SVerticalBox);
	}

	void RefreshAbilityList()
	{
		if (!AbilityListBox.IsValid())
		{
			return;
		}

		AbilityListBox->ClearChildren();

		if (MenuMode == EBattleMenuMode::Items)
		{
			AbilityListBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Items (not implemented)")))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.f))
			];
			return;
		}

		for (int32 Index = 0; Index < Abilities.Num(); ++Index)
		{
			UAbilityDataAsset* Ability = Abilities[Index].Get();
			if (!Ability)
			{
				continue;
			}

			AbilityListBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
			[
				SNew(SButton)
				.OnClicked(this, &SProjectVegaBattleMenu::HandleAbilityClicked, Index)
				.OnHovered(FSimpleDelegate::CreateSP(this, &SProjectVegaBattleMenu::HandleAbilityHovered, Index))
				.OnUnhovered(FSimpleDelegate::CreateSP(this, &SProjectVegaBattleMenu::HandleAbilityUnhovered, Index))
				.ContentPadding(FMargin(10.f, 4.f))
				[
					SNew(STextBlock)
					.Text(BuildAbilityText(Ability))
					.ColorAndOpacity(this, &SProjectVegaBattleMenu::GetAbilityColorForIndex, Index)
					.ShadowOffset(this, &SProjectVegaBattleMenu::GetAbilityShadowOffsetForIndex, Index)
					.ShadowColorAndOpacity(this, &SProjectVegaBattleMenu::GetAbilityShadowColorForIndex, Index)
				]
			];
		}
	}

	FSlateColor GetAbilityColorForIndex(int32 Index) const
	{
		UAbilityDataAsset* Ability = Abilities.IsValidIndex(Index) ? Abilities[Index].Get() : nullptr;
		FLinearColor Base = BuildAbilityColor(Ability).GetSpecifiedColor();
		if (Index == HoveredAbilityIndex)
		{
			FLinearColor Glow(0.2f, 1.f, 0.9f, 1.f);
			return FSlateColor((Base + Glow * 0.35f).GetClamped());
		}
		return FSlateColor(Base);
	}

	FSlateColor GetCommandColorForIndex(int32 Index) const
	{
		FLinearColor Base(0.8f, 0.9f, 1.f, 1.f);
		if (Index == HoveredCommandIndex)
		{
			FLinearColor Glow(0.2f, 1.f, 0.9f, 1.f);
			return FSlateColor((Base + Glow * 0.35f).GetClamped());
		}
		return FSlateColor(Base);
	}

	FVector2D GetCommandShadowOffsetForIndex(int32 Index) const
	{
		return (Index == HoveredCommandIndex) ? FVector2D(0.5f, 0.5f) : FVector2D::ZeroVector;
	}

	FLinearColor GetCommandShadowColorForIndex(int32 Index) const
	{
		return (Index == HoveredCommandIndex) ? FLinearColor(0.1f, 1.f, 0.9f, 0.45f) : FLinearColor::Transparent;
	}

	FVector2D GetAbilityShadowOffsetForIndex(int32 Index) const
	{
		UAbilityDataAsset* Ability = Abilities.IsValidIndex(Index) ? Abilities[Index].Get() : nullptr;
		if (Index == HoveredAbilityIndex)
		{
			return FVector2D(0.5f, 0.5f);
		}
		return BuildAbilityShadowOffset(Ability);
	}

	FLinearColor GetAbilityShadowColorForIndex(int32 Index) const
	{
		UAbilityDataAsset* Ability = Abilities.IsValidIndex(Index) ? Abilities[Index].Get() : nullptr;
		FLinearColor Base = BuildAbilityShadowColor(Ability);
		if (Index == HoveredAbilityIndex)
		{
			FLinearColor Glow(0.1f, 1.f, 0.9f, 0.45f);
			return (Base + Glow).GetClamped();
		}
		return Base;
	}

	FText BuildAbilityText(UAbilityDataAsset* Ability) const
	{
		if (!Ability)
		{
			return FText::GetEmpty();
		}

		FString Name = Ability->AbilityName.ToString();
		if (Ability->AbilitySpecial == EAbilitySpecialType::Overdrive)
		{
			Name += TEXT(" [OVERDRIVE]");
		}
		else if (Ability->AbilitySpecial == EAbilitySpecialType::Herculean)
		{
			Name += TEXT(" [HERCULEAN]");
		}
		return FText::FromString(Name);
	}

	FSlateColor BuildAbilityColor(UAbilityDataAsset* Ability) const
	{
		if (!Ability)
		{
			return FSlateColor(FLinearColor::White);
		}

		if (Ability->AbilitySpecial == EAbilitySpecialType::Overdrive)
		{
			return FSlateColor(FLinearColor(1.f, 0.6f, 0.1f, 1.f));
		}
		if (Ability->AbilitySpecial == EAbilitySpecialType::Herculean)
		{
			return FSlateColor(FLinearColor(0.85f, 0.4f, 1.f, 1.f));
		}

		switch (Ability->AbilityCategory)
		{
			case EAbilityCategory::Offensive:
				return FSlateColor(FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			case EAbilityCategory::Defensive:
				return FSlateColor(FLinearColor(0.3f, 1.f, 0.5f, 1.f));
			case EAbilityCategory::Utility:
				return FSlateColor(FLinearColor(0.3f, 0.6f, 1.f, 1.f));
			case EAbilityCategory::Hack:
				return FSlateColor(FLinearColor(1.f, 0.9f, 0.3f, 1.f));
			default:
				return FSlateColor(FLinearColor::White);
		}
	}

	FVector2D BuildAbilityShadowOffset(UAbilityDataAsset* Ability) const
	{
		if (Ability && Ability->AbilitySpecial != EAbilitySpecialType::None)
		{
			return FVector2D(1.f, 1.f);
		}
		return FVector2D::ZeroVector;
	}

	FLinearColor BuildAbilityShadowColor(UAbilityDataAsset* Ability) const
	{
		if (Ability && Ability->AbilitySpecial == EAbilitySpecialType::Overdrive)
		{
			return FLinearColor(1.f, 0.4f, 0.1f, 0.9f);
		}
		if (Ability && Ability->AbilitySpecial == EAbilitySpecialType::Herculean)
		{
			return FLinearColor(0.6f, 0.2f, 1.f, 0.9f);
		}
		return FLinearColor::Transparent;
	}

	FReply HandleAbilityClicked(int32 Index)
	{
		if (Abilities.IsValidIndex(Index))
		{
			if (UAbilityDataAsset* Ability = Abilities[Index].Get())
			{
				if (OnAbilityPicked.IsBound())
				{
					OnAbilityPicked.Execute(Ability);
				}
			}
		}
		return FReply::Handled();
	}

	void HandleAbilityHovered(int32 Index)
	{
		HoveredAbilityIndex = Index;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void HandleAbilityUnhovered(int32 Index)
	{
		if (HoveredAbilityIndex == Index)
		{
			HoveredAbilityIndex = INDEX_NONE;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

	void HandleCommandHovered(int32 Index)
	{
		HoveredCommandIndex = Index;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void HandleCommandUnhovered(int32 Index)
	{
		if (HoveredCommandIndex == Index)
		{
			HoveredCommandIndex = INDEX_NONE;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

	FReply HandleAbilities()
	{
		MenuMode = EBattleMenuMode::Abilities;
		RefreshAbilityList();
		return FReply::Handled();
	}

	FReply HandleItems()
	{
		MenuMode = EBattleMenuMode::Items;
		RefreshAbilityList();
		return FReply::Handled();
	}

	FReply HandleEndTurn()
	{
		if (OnEndTurn.IsBound())
		{
			OnEndTurn.Execute();
		}
		return FReply::Handled();
	}

private:
	TArray<TWeakObjectPtr<UAbilityDataAsset>> Abilities;
	int32 HoveredAbilityIndex = INDEX_NONE;
	int32 HoveredCommandIndex = INDEX_NONE;
	TSharedPtr<SVerticalBox> AbilityListBox;
	TSharedPtr<SBorder> RightPanel;
	EBattleMenuMode MenuMode = EBattleMenuMode::Commands;
	FOnAbilityPickedNative OnAbilityPicked;
	FOnEndTurnNative OnEndTurn;
};

void UProjectVegaBattleMenuWidget::SetAbilities(const TArray<UAbilityDataAsset*>& InAbilities)
{
	CachedAbilities.Reset();
	for (UAbilityDataAsset* Ability : InAbilities)
	{
		CachedAbilities.Add(Ability);
	}

	if (BattleMenu.IsValid())
	{
		BattleMenu->SetAbilities(CachedAbilities);
	}
}

TSharedRef<SWidget> UProjectVegaBattleMenuWidget::RebuildWidget()
{
	BattleMenu = SNew(SProjectVegaBattleMenu)
		.OnAbilityPicked(FOnAbilityPickedNative::CreateUObject(this, &UProjectVegaBattleMenuWidget::HandleAbilityPicked))
		.OnEndTurn(FOnEndTurnNative::CreateUObject(this, &UProjectVegaBattleMenuWidget::HandleEndTurn));

	BattleMenu->SetAbilities(CachedAbilities);
	return BattleMenu.ToSharedRef();
}

void UProjectVegaBattleMenuWidget::HandleAbilityPicked(UAbilityDataAsset* Ability)
{
	OnAbilitySelected.Broadcast(Ability);
}

void UProjectVegaBattleMenuWidget::HandleEndTurn()
{
	OnEndTurnRequested.Broadcast();
}
