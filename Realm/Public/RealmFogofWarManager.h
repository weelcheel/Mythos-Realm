#pragma once

#include "RealmFogofWarManager.generated.h"

class AGameCharacter;
class ARealmPlayerController;

UCLASS()
class URealmFogofWarManager : public UObject
{
	GENERATED_UCLASS_BODY()

protected:

	/* array of game characters that use this manager */
	TArray<AGameCharacter*> teamCharacters;

	/* array of player controllers that use the vision */
	//TArray<ARealmPlayerController*> teamPlayers;

	/* timer that calls for characters on the team to calculate visibiltiy */
	FTimerHandle visibilityTimer;

	/* tell all of the characters to calculate visibility */
	void CalculateTeamVisibility();

	UFUNCTION()
	void OnRep_EnemySightList();

public:

	/* team this sight manager is for , -1 for all characters */
	int32 teamIndex;

	/* local player using this fog of war managaer */
	UPROPERTY(replicated)
	ARealmPlayerController* playerOwner;

	/* array of units that this player can see and is used for updating vision in-game */
	UPROPERTY(ReplicatedUsing=OnRep_EnemySightList)
	TArray<AGameCharacter*> enemySightList;

	/* called whenever we need to add a character to the manager */
	void AddCharacterToManager(AGameCharacter* newCharacter);

	/* called whenever we need to remove a character from the manager */
	void RemoveCharacterFromManager(AGameCharacter* oldCharacter);

	/* add a player to the manager */
	void AddPlayerToManager(ARealmPlayerController* newPlayer);

	/* starts the timer for calculating unit visibility */
	void StartCalculatingVisibility();

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
};