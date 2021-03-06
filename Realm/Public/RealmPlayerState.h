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

	/* amount of kills this player has */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Stats)
	int32 playerKills;

	/* amount of kills this player has */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Stats)
	int32 playerDeaths;

	/* amount of assists this player has got this game */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Stats)
	int32 playerAssists;

	/* total amount of credits the player has earned this game */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Stats)
	int32 playerTotalIncome;

	/* amount of creeps/minions this player last hit */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Stats)
	int32 playerCreepScore;

	/* amount of player's mythos points for UI */
	UPROPERTY(replicated, BlueprintReadWrite, Category = Stats)
	int32 mythosPoints;

	UFUNCTION(BlueprintCallable, Category = Team)
	int32 GetTeamIndex() const;

	void SetTeamIndex(int32 newTeam);

	/** broadcast death for this player state to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class ARealmPlayerState* KillerPlayerState, APawn* killerPawn);

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