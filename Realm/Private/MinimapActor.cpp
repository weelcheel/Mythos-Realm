#include "Realm.h"
#include "MinimapActor.h"
#include "PlayerHUD.h"

AMinimapActor::AMinimapActor(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void AMinimapActor::ReceiveCharacterVisibilityUpdate(const TArray<AGameCharacter*>& visibleCharacters)
{
	TArray<FMinimapEntry> entries;

	for (int32 i = 0; i < visibleCharacters.Num(); i++)
	{
		if (!IsValid(visibleCharacters[i]))
			continue;

		FMinimapEntry entry;
		entry.character = visibleCharacters[i];

		FVector2D relPos;
		relPos.X = (visibleCharacters[i]->GetActorLocation().Y - GetActorLocation().Y) / mapSize.X;
		relPos.Y = (GetActorLocation().X - visibleCharacters[i]->GetActorLocation().X) / mapSize.X;

		entry.relativePosition.X = relPos.Size() * FMath::Cos(FMath::Atan2(relPos.Y, relPos.X));
		entry.relativePosition.Y = relPos.Size() * FMath::Sin(FMath::Atan2(relPos.Y, relPos.X));

		entries.Add(entry);
	}

	if (IsValid(hud))
		hud->OnMinimapVisibleCharactersUpdate(entries);
}