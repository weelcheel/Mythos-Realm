#pragma once

#include "RealmFogofWarManager.generated.h"

class AGameCharacter;
class ARealmPlayerController;

UCLASS()
class ARealmFogofWarManager : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/* array of game characters that use this manager */
	TArray<AGameCharacter*> teamCharacters;

	/* array of player controllers that use the vision */
	TArray<ARealmPlayerController*> teamPlayers;

	/* timer that calls for characters on the team to calculate visibiltiy */
	FTimerHandle visibilityTimer;

	virtual void BeginPlay() override;

	/* tell all of the characters to calculate visibility */
	void CalculateTeamVisibility();

public:

	/* team this sight manager is for , -1 for all characters */
	int32 teamIndex;

	/* called whenever we need to add a character to the manager */
	void AddCharacterToManager(AGameCharacter* newCharacter);

	/* called whenever we need to remove a character from the manager */
	void RemoveCharacterFromManager(AGameCharacter* oldCharacter);

	/* add a player to the manager */
	void AddPlayerToManager(ARealmPlayerController* newPlayer);
};