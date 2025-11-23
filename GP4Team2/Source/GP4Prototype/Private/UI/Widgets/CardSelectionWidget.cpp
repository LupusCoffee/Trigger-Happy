#include "UI/Widgets/CardSelectionWidget.h"

void UCardSelectionWidget::PopulateCards_Implementation(const TArray<UUpgradeCardData*>& Cards)
{
	CurrentCards.Reset();
	for (UUpgradeCardData* C : Cards)
	{
		CurrentCards.Add(C);
	}
	OnPopulateCardsVisuals(Cards);
}

void UCardSelectionWidget::HandleCardClicked(UUpgradeCardData* Selected)
{
	LastSelectedCard = Selected;
	OnCardPicked.Broadcast(Selected);
	OnCardPickedBP(Selected);
}

