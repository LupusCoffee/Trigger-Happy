#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "CardSlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardClickedSignature, UUpgradeCardData*, CardData);

UCLASS(BlueprintType, Blueprintable)
class GP4PROTOTYPE_API UCardSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// The card data this slot represents
	UPROPERTY(BlueprintReadOnly, Category="Card")
	TObjectPtr<UUpgradeCardData> CardData;

	// Optional: owning attribute component for context (e.g., to check rules)
	UPROPERTY(BlueprintReadWrite, Category="Card")
	UAttributeComponent* OwningAttributeComponent = nullptr;

	// Initialize this slot from a card; calls the BP event for visual setup
	UFUNCTION(BlueprintCallable, Category="Card")
	void InitializeCard(UUpgradeCardData* InCardData, UAttributeComponent* InOwningAttributeComponent = nullptr);

	// Implement in BP to set up any visuals/text/icons for this card
	UFUNCTION(BlueprintImplementableEvent, Category="Card")
	void OnSetupCard(UUpgradeCardData* InCardData, UAttributeComponent* InOwningAttributeComponent);

	// Call from BP (e.g., bound to a Button OnClicked) to signal selection
	UFUNCTION(BlueprintCallable, Category="Card")
	void NotifyCardClicked();

	// External listeners can bind to know which card was chosen
	UPROPERTY(BlueprintAssignable, Category="Card")
	FOnCardClickedSignature OnCardClicked;
};
