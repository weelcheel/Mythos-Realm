#include "Realm.h"
#include "MinimapActor.h"
#include "PlayerHUD.h"

AMinimapActor::AMinimapActor(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	mapExtents = objectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("mapExtents"));
	mapExtents->SetSphereRadius(1024.f);
}

void AMinimapActor::BeginPlay()
{
	Super::BeginPlay();

	mapCenter.X = GetActorLocation().X;
	mapCenter.Y = GetActorLocation().Y;

	mapSizeMin.X = mapCenter.X - mapExtents->GetUnscaledSphereRadius();
	mapSizeMax.X = mapCenter.X + mapExtents->GetUnscaledSphereRadius();
	mapSizeMin.Y = mapCenter.Y - mapExtents->GetUnscaledSphereRadius();
	mapSizeMax.Y = mapCenter.Y + mapExtents->GetUnscaledSphereRadius();
}

void AMinimapActor::ReceiveCharacterVisibilityUpdate(const TArray<AGameCharacter*>& visibleCharacters)
{
	/*TArray<FMinimapEntry> entries;

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
		hud->OnMinimapVisibleCharactersUpdate(entries);*/
}

float AMinimapActor::GetRadianHeading() const
{
	FVector vec;
	FRotator rot;
	float radians;

	//only need a vector with this actor's 
	rot.Yaw = GetActorRotation().Yaw;
	vec = rot.Vector();

	radians = vec.HeadingAngle();
	radians = FMath::UnwindRadians(radians);

	while (radians < 0.f)
		radians += PI * 2.0f;

	return radians;
}

float AMinimapActor::GetDegreeHeading() const
{
	return FMath::RadiansToDegrees(GetRadianHeading());
}