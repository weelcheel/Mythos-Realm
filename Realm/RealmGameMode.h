// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "RealmGameMode.generated.h"

class AMod;
class ARealmEnabler;
class APlayerCharacter;
class ARealmPlayerState;
class AGameCharacter;

UENUM()
enum class EGameStatus : uint8
{
	GS_Pregame,
	GS_CharacterSelect,
	GS_Ingame,
	GS_Postgame,
	GS_MAX
};

USTRUCT()
struct FTeam
{
	GENERATED_USTRUCT_BODY()

	/* array of player state references to the players on the team */
	TArray<ARealmPlayerState*> players;
};

/**
 * 
 */
UCLASS()
class REALM_API ARealmGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

protected:

	/* items that are offered in this game type's store */
	UPROPERTY(EditDefaultsOnly, Category = Store)
	TArray<TSubclassOf<AMod> > storeMods;

	/* number of teams in the game */
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	int32 teamCount;

	/* how big the teams can be */
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	int32 teamSizeMax;

	/* how big each of the teams needs to be for the game to start */
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	int32 teamSizeMin;

	/* the array of characters this gametype can use */
	UPROPERTY(EditDefaultsOnly, Category = Characters)
	TArray<TSubclassOf<APlayerCharacter> > availableCharacters;

	/* amount of credits each player kill is worth */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 playerKillWorth;

	/* amount of credits each player gets per second of the game */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 ambientCreditIncome;

	/* the array of characters the players have elected to ban */
	TArray<TSubclassOf<APlayerCharacter> > bannedCharacters;

	/* whether or not there is friendly fire */
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	bool bFriendlyFire;

	/* whether or not the game is over */
	bool bGameWinner = false;

	/* what state this game mode is currently in */
	EGameStatus gameStatus;

	/* array of teams in the game, usually 2 */
	TArray<FTeam> teams;

	virtual void BeginPlay() override;

	/* each time a player logs in, check to see if we can start the game */
	void CheckForCharacterSelect();

	/* transition the game to character select */
	void TransitionToCharacterSelect();

	/* check for all players to have a chosen class */
	void CheckForAllCharactersSelected();

	/* start the match and start minion spawning */
	virtual void StartMatch() override;

	/* start credit income */
	void StartCreditIncome();

public:

	/* get the store items for this game */
	UFUNCTION(BlueprintCallable, Category = Store)
	void GetStoreMods(TArray<TSubclassOf<AMod> >& modsToSell);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	/* called when an enabler is destroyed to end the game */
	void EnablerDestroyed(ARealmEnabler* enablerDestroyed);

	/* can damage friendly units */
	bool CanDamageFriendlies() const;

	/* called when a player selects a character */
	UFUNCTION(BlueprintCallable, Category = Store)
	void PlayerSelectedCharacter(ARealmPlayerController* player, TSubclassOf<APlayerCharacter> characterClass);

	/* called when a player dies */
	void PlayerDied(ARealmPlayerController* killedPlayer, ARealmPlayerController* playerKiller);
};
