#include "Realm.h"
#include "RealmFogofWarManager.h"
#include "GameCharacter.h"
#include "PlayerCharacter.h"
#include "RealmPlayerController.h"
#include "UnrealNetwork.h"

URealmFogofWarManager::URealmFogofWarManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void URealmFogofWarManager::StartCalculatingVisibility()
{
	if (IsValid(playerOwner) && playerOwner->HasAuthority())
		playerOwner->GetWorldTimerManager().SetTimer(visibilityTimer, this, &URealmFogofWarManager::CalculateTeamVisibility, (0.15f), true);
}

void URealmFogofWarManager::CalculateTeamVisibility()
{
	if (!IsValid(playerOwner))
		return;

	teamCharacters.Empty();
	enemySightList.Empty();

	//get which characters are on our team
	for (TActorIterator<AGameCharacter> chr(playerOwner->GetWorld()); chr; ++chr)
	{
		AGameCharacter* gc = *chr;
		if (IsValid(gc) && gc->GetTeamIndex() == teamIndex && gc->IsAlive())
			teamCharacters.AddUnique(gc);

		if (IsValid(gc) && gc->GetTeamIndex() == teamIndex) //always have sight of friendly units and the last unit to do recent damage to them
		{
			enemySightList.AddUnique(gc);
			if (IsValid(gc->lastDamagingCharacter))
				enemySightList.AddUnique(gc->lastDamagingCharacter);
		}
	}
	/*for (TActorIterator<APlayerCharacter> chr(GetWorld()); chr; ++chr)
	{
		APlayerCharacter* gc = *chr;
		if (IsValid(gc) && gc->GetTeamIndex() == teamIndex && gc->IsAlive())
			teamCharacters.AddUnique(gc);
	}*/

	//go through all of the players and get their visibility data. this data results in a list of enemy teams that can see that specific unit and replicates to all clients
	for (int32 i = 0; i < teamCharacters.Num(); i++)
	{
		//get their sight data
		teamCharacters[i]->CalculateVisibility(enemySightList);
	}
}

void URealmFogofWarManager::AddCharacterToManager(AGameCharacter* newCharacter)
{
	if (IsValid(newCharacter))
		teamCharacters.AddUnique(newCharacter);
}

void URealmFogofWarManager::RemoveCharacterFromManager(AGameCharacter* oldCharacter)
{
	if (IsValid(oldCharacter))
		teamCharacters.Remove(oldCharacter);
}

void URealmFogofWarManager::AddPlayerToManager(ARealmPlayerController* newPlayer)
{
	//if (IsValid(newPlayer))
		//teamPlayers.AddUnique(newPlayer);
}

void URealmFogofWarManager::OnRep_EnemySightList()
{
	if (!IsValid(playerOwner))
		return;

	for (TActorIterator<AGameCharacter> chr(playerOwner->GetWorld()); chr; ++chr)
	{
		AGameCharacter* gc = *chr;
		if (!IsValid(gc))
			continue;

		gc->SetActorHiddenInGame(!enemySightList.Contains(gc));
	}
}

void URealmFogofWarManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	DOREPLIFETIME(URealmFogofWarManager, enemySightList);
	DOREPLIFETIME(URealmFogofWarManager, playerOwner);
}