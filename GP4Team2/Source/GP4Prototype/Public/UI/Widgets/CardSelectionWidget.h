#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "CardSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPickedSignature, UUpgradeCardData*, CardData);

UCLASS(BlueprintType, Blueprintable)
class GP4PROTOTYPE_API UCardSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Cards currently represented by this widget (after PopulateCards)
	UPROPERTY(BlueprintReadOnly, Category="Card")
	TArray<TObjectPtr<UUpgradeCardData>> CurrentCards;

	// Optional: last selection for convenience
	UPROPERTY(BlueprintReadOnly, Category="Card")
	TObjectPtr<UUpgradeCardData> LastSelectedCard = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="Card")
	UAttributeComponent* OwningAttributeComponent = nullptr;

	// Called from BP to populate the widget with cards. C++ runs first, then BP override if provided.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Card")
	void PopulateCards(const TArray<UUpgradeCardData*>& Cards);

	// BP can implement this to actually build the UI (spawn slots, bind buttons, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category="Card")
	void OnPopulateCardsVisuals(const TArray<UUpgradeCardData*>& Cards);

	// When a card slot/button is clicked in BP, call this; it will broadcast and also fire a BP event.
	UFUNCTION(BlueprintCallable, Category="Card")
	void HandleCardClicked(UUpgradeCardData* Selected);

	// C++/BP listeners can subscribe to this to apply the chosen card
	UPROPERTY(BlueprintAssignable, Category="Card")
	FOnCardPickedSignature OnCardPicked;

	// BP event for local UI reactions (close panel, play effects, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category="Card")
	void OnCardPickedBP(UUpgradeCardData* Selected);
};
