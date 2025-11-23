#include "UI/Widgets/CardSlotWidget.h"

void UCardSlotWidget::InitializeCard(UUpgradeCardData* InCardData, UAttributeComponent* InOwningAttributeComponent)
{
	CardData = InCardData;
	OwningAttributeComponent = InOwningAttributeComponent;
	OnSetupCard(InCardData, InOwningAttributeComponent);
}

void UCardSlotWidget::NotifyCardClicked()
{
	OnCardClicked.Broadcast(CardData);
}

