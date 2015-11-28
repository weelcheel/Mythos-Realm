#pragma once

#include "GameFramework/PlayerState.h"
#include "RealmPlayerState.generated.h"

class APlayerCharacter;

UCLASS()
class ARealmPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()

protected:

	/* what team this player is on */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Team)
	int32 teamIndex;

	/* what the player's selected character is */
	UPROPERTY(replicated)
	TSubclassOf<APlayerCharacter> chosenCharacterClass;

	/* the index to ID which player this on the team (which player this is in the teams array) */
	UPROPERTY(replicated)
	int32 teamPlayerIndex;

public:

	UFUNCTION(BlueprintCallable, Category = Team)
	int32 GetTeamIndex() const;

	void SetTeamIndex(int32 newTeam);

	TSubclassOf<APlayerCharacter> GetChosenCharacterClass() const
	{
		return chosenCharacterClass;
	}

	void SetChosenCharacterClass(TSubclassOf<APlayerCharacter> cClass)
	{
		chosenCharacterClass = cClass;
	}

	UFUNCTION(BlueprintCallable, Category = Team)
	int32 GetTeamPlayerIndex() const
	{
		return teamPlayerIndex;
	}

	UFUNCTION(BlueprintCallable, Category = Team)
	void SetTeamPlayerIndex(int32 newIndex)
	{
		teamPlayerIndex = newIndex;
	}
};