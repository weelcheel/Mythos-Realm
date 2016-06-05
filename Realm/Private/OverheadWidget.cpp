#include "Realm.h"
#include "OverheadWidget.h"
#include "GameCharacter.h"

UOverheadWidget::UOverheadWidget(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void UOverheadWidget::SetParentCharacter(AGameCharacter* newCharacter)
{
	parentCharacter = newCharacter;
}

void UOverheadWidget::Tick(FGeometry MyGeometry, float InDeltaTime)
{
	Super::Tick(MyGeometry, InDeltaTime);

	if (!IsValid(parentCharacter))
		RemoveFromParent();
}