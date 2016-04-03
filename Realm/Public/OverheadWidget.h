#pragma once

#include "Runtime/UMG/Public/UMG.h"
#include "SlateBasics.h"
#include "OverheadWidget.generated.h"

class AGameCharacter;

UCLASS()
class UOverheadWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()

protected:

	/* game character this widget is referencing */
	UPROPERTY(BlueprintReadOnly, Category=Character)
	AGameCharacter* parentCharacter;

public:

	/* sets the parent character */
	void SetParentCharacter(AGameCharacter* newCharacter);

	/* event that lets the blueprint decide the z order for the widget and add it to the viewport */
	UFUNCTION(BlueprintImplementableEvent, Category = UI)
	void AddOverheadWidgetToViewport();
};