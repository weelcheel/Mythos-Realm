// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "Chat.h"
#include "RealmGameMode.generated.h"

class AMod;
class ARealmEnabler;
class APlayerCharacter;
class ARealmPlayerState;
class AGameCharacter;
class URealmFogofWarManager;
class ARealmObjective;
class ALaneManager;

struct FRealmChatEntry;

UENUM()
enum class EGameStatus : uint8
{
	GS_Pregame,
	GS_CharacterSelect,
	GS_Ingame,
	GS_Postgame,
	GS_MAX
};

UENUM(BlueprintType)
enum class ECharacterSelectMode : uint8
{
	CSM_Any UMETA(DisplayName = "No Mythos Limits"),
	CSM_OnePerTeam UMETA(DisplayName = "One Mythos per Team"),
	CSM_OnePerGame UMETA(DisplayName = "One Mythos per Match"),
	CSM_MAX UMETA(Hidden)
};

USTRUCT()
struct FTeam
{
	GENERATED_USTRUCT_BODY()

	/* array of player state references to the players on the team */
	TArray<ARealmPlayerState*> players;

	/* avreage level of the players on the team */
	int32 averageTeamLevel;

	/* what characters have been selected this match for this team */
	TArray<TSubclassOf<APlayerCharacter> > chosenCharacters;
};

/**
 * 
 */
UCLASS()
class REALM_API ARealmGameMode : public AGameMode
{
	friend class URealmGameInstance;
	friend class ARealmPlayerController;
	friend class URealmFogofWarManager;

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

	/* expected numbere of players in the game */
	int32 expectedPlayerCount;

	/* the array of characters this gametype can use */
	UPROPERTY(EditDefaultsOnly, Category = Characters)
	TArray<TSubclassOf<APlayerCharacter> > availableCharacters;

	/* amount of credits each player kill is worth */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 playerKillWorth;

	/* amount of credits each player gets per second of the game */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 ambientCreditIncome;

	/* amount of credits players normally start out with */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 startingCreditCount;

	/* amount of skill points each player starts with */
	UPROPERTY(EditDefaultsOnly, Category = Kills)
	int32 startingSkillPoints;

	/* amount of time before raiders start spawning in the game */
	UPROPERTY(EditDefaultsOnly, Category = Raiders)
	float raiderSpawnDelay;

	/* amount of time between the death of the last spawned raider and when the next one spawns */
	UPROPERTY(EditDefaultsOnly, Category = Raiders)
	float raiderRespawnDelay;

	/* what the restrictions for mythos select are for this game mode */
	UPROPERTY(EditDefaultsOnly, Category = SelectMode)
	ECharacterSelectMode selectMode;

	/* timer for raider respawn */
	FTimerHandle raiderSpawnTimer;

	/* called when a raider needs to spawn */
	void OnRaiderSpawn();
	/* called to calculate the next raider spawn point and type and announces it to the players */
	void CalculateNextRaiderSpawn();
	/* whether or not this was the raider's first death */
	bool bRaidersFirstDeath = false;

	/* where the raider is spawning next */
	ALaneManager* nextRaiderSpawningLane;
	/* what type of raider to spawn next */
	TSubclassOf<ARaiderCharacter> nextRaiderType;

	/* what types of raiders can spawn in this gametype */
	UPROPERTY(EditDefaultsOnly, Category = Raiders)
	TArray<TSubclassOf<ARaiderCharacter> > raiderTypes;

	/* blueprint hook to spawn indicators and announcements for when a new spawn is calculated for a raider */
	UFUNCTION(BlueprintImplementableEvent, Category = Raiders)
	void OnNextRaiderSpawnCalculated(TSubclassOf<ARaiderCharacter> raiderType, ALaneManager* lane);

	/* whether or not this game counts towards player's competitive rank */
	bool bRankedGame = false;

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

	/* set the winner of the game */
	int32 winningTeamIndex;

	/* user ids of all of the players in the game (for end game) */
	TArray<FString> endgameUserids;

	/* team indices of all player getting skill updates (matches with endgameUserids)*/
	TArray<int32> endgameTeams;

	/* end game user id check */
	FTimerHandle useridcheck;

	/* amount of time it takes for the games minions to level up ambiently */
	UPROPERTY(EditDefaultsOnly, Category = MinionLevel)
	float ambientLevelUpTime;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	/* calculate the credit value of a player kill */
	int32 CalculatePlayerKillValue(AController* killedPlayer, AController* killerPlayer);

	/* end match */
	virtual void EndRealmMatch();

	/* calculate end game variables to send to master server */
	void CalculateEndgame();

	/* check for userid values calculated */
	void CheckForEndgameIDs();

	/* tell the game instance to send the end game to the master server */
	void ReportEndGame();

	/* server received end game stats so show clients post game screen */
	void BeginPostGame();

public:

	/* sight manager for teams */
	UPROPERTY(BlueprintReadOnly, Category = Sight)
	URealmFogofWarManager* fogOfWar;

	/* pool of characters that are currently available for sight in this game */
	UPROPERTY(BlueprintReadOnly, Category = Sight)
	TArray<AGameCharacter*> availableSightUnits;

	/* get the store items for this game */
	UFUNCTION(BlueprintCallable, Category = Store)
	void GetStoreMods(TArray<TSubclassOf<AMod> >& modsToSell);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	/* called when an enabler is destroyed to end the game */
	void EnablerDestroyed(ARealmEnabler* enablerDestroyed, int32 winningTeam);

	/* can damage friendly units */
	bool CanDamageFriendlies() const;

	/* called when a player selects a character */
	UFUNCTION(BlueprintCallable, Category = Store)
	void PlayerSelectedCharacter(ARealmPlayerController* player, TSubclassOf<APlayerCharacter> characterClass);

	/* called when a player dies */
	void PlayerDied(AController *killedPlayer, AController* playerKiller, APawn* killerPawn);

	/* called when an objective is destroyed */
	void ObjectiveDestroyed(ARealmObjective* destroyedObjective, APawn* killerPawn);

	/* find player start */
	virtual AActor* FindPlayerStart(AController* Player, const FString& IncomingName = TEXT(""));

	virtual void RestartPlayer(class AController* NewPlayer);

	void ReceiveEndgameStats(const FString& userid, int32 teamIndex);

	/* called whenever a player levels up so we can adjust minion levels */
	void PlayerLeveledUp();

	/* called every period of time to level up the teams minions */
	void AmbientGameLevelUp();

	/* get the starting amount of credits */
	int32 GetStartingCredits() const
	{
		return startingCreditCount;
	}

	/* called when a raider dies */
	void OnRaiderDeath(bool bDespawned = false);
};
