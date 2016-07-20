#pragma once

#include "RealmFogofWarManager.generated.h"

class AGameCharacter;
class ARealmPlayerController;
class ARealmGameMode;

USTRUCT()
struct FTeamSightList
{
	GENERATED_USTRUCT_BODY()

	TArray<AGameCharacter*> sightList;
};

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

public:

	/* team this sight manager is for , -1 for all characters */
	int32 teamIndex;

	/* local player using this fog of war managaer */
	UPROPERTY()
	ARealmPlayerController* playerOwner;

	/* game mode that is using this manager if there is no player owner */
	UPROPERTY()
	ARealmGameMode* gameOwner;

	/* array of units that this player can see and is used for updating vision in-game */
	UPROPERTY()
	TArray<FTeamSightList> enemySightLists;

	/* available units to process for visibility */
	TArray<AGameCharacter*> availableUnits;

	/* called whenever we need to add a character to the manager */
	void AddCharacterToManager(AGameCharacter* newCharacter);

	/* called whenever we need to remove a character from the manager */
	void RemoveCharacterFromManager(AGameCharacter* oldCharacter);

	/* add a player to the manager */
	void AddPlayerToManager(ARealmPlayerController* newPlayer);

	/* starts the timer for calculating unit visibility */
	void StartCalculatingVisibility();

	/* gets whether or not one character can see another */
	UFUNCTION(BlueprintCallable, Category = Sight)
	bool CanUnitSeeOther(AGameCharacter* originUnit, AGameCharacter* testUnit) const;

	/* override begin destroy to stop the visibility calculating thread */
	virtual void BeginDestroy() override;
};

/* multi-threaded class to calculate visibility for the fog of war manager, as running calculateVisibility on the gameThread causes a MASSIVE fps drop*/
class FGameVisibilityWorker : public FRunnable
{
	/* runnable thread */
	FRunnableThread* Thread;

	/* the fog of war manager we're running this thread for */
	URealmFogofWarManager* fogOfWar;

	/* stop the thread if the manager is not valid anymore */
	FThreadSafeCounter stopTaskCounter;

	/* pointer to the array of sight lists */
	TArray<FTeamSightList>* enemySightLists;

	/* calculate visibilities */
	void CalculateVisibilities();

public:

	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static FGameVisibilityWorker* runnable;

	FGameVisibilityWorker(URealmFogofWarManager* inFoW, TArray<FTeamSightList>& sightLists);
	virtual ~FGameVisibilityWorker();

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	void EnsureCompletion();

	static FGameVisibilityWorker* WorkerInit(URealmFogofWarManager* inFoW);
	static void Shutdown();
};