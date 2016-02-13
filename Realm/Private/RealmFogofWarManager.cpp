#include "Realm.h"
#include "RealmFogofWarManager.h"
#include "GameCharacter.h"
#include "PlayerCharacter.h"
#include "RealmPlayerController.h"

ARealmFogofWarManager::ARealmFogofWarManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void ARealmFogofWarManager::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(visibilityTimer, this, &ARealmFogofWarManager::CalculateTeamVisibility, 0.15f, true);
}

void ARealmFogofWarManager::CalculateTeamVisibility()
{
	TArray<AGameCharacter*> newSightArray;

	//get which characters are on our team
	for (TActorIterator<AGameCharacter> chr(GetWorld()); chr; ++chr)
	{
		AGameCharacter* gc = *chr;
		if (IsValid(gc) && gc->GetTeamIndex() == teamIndex && gc->IsAlive())
			teamCharacters.AddUnique(gc);
	}
	for (TActorIterator<APlayerCharacter> chr(GetWorld()); chr; ++chr)
	{
		APlayerCharacter* gc = *chr;
		if (IsValid(gc) && gc->GetTeamIndex() == teamIndex && gc->IsAlive())
			teamCharacters.AddUnique(gc);
	}

	//remove bad team characters
	for (int32 i = 0; i < teamCharacters.Num(); i++)
	{
		if (!IsValid(teamCharacters[i]) || (IsValid(teamCharacters[i]) && !teamCharacters[i]->IsAlive()))
			RemoveCharacterFromManager(teamCharacters[i]);
	}

	//go through all of the players and get their visibility data
	for (int32 i = 0; i < teamCharacters.Num(); i++)
	{
		//get their sight data
		teamCharacters[i]->CalculateVisibility(newSightArray);
	}

	for (int32 i = 0; i < teamPlayers.Num(); i++)
	{
		teamPlayers[i]->ClientSetVisibleCharacters(newSightArray);
	}
}

void ARealmFogofWarManager::AddCharacterToManager(AGameCharacter* newCharacter)
{
	if (IsValid(newCharacter))
		teamCharacters.AddUnique(newCharacter);
}

void ARealmFogofWarManager::RemoveCharacterFromManager(AGameCharacter* oldCharacter)
{
	if (IsValid(oldCharacter))
		teamCharacters.Remove(oldCharacter);
}

void ARealmFogofWarManager::AddPlayerToManager(ARealmPlayerController* newPlayer)
{
	if (IsValid(newPlayer))
		teamPlayers.AddUnique(newPlayer);
}