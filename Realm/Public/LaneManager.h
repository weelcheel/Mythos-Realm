#pragma once

#include "LaneManager.generated.h"

class AMinionCharacter;
class ARealmEnabler;

UCLASS()
class ALaneManager : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/* timer to handle the spawning of minions */
	FTimerHandle minionTimer, minionTimer2;

	/* timer to handle the time inbetween waves */
	FTimerHandle waveTimer;

	/* whether or not the enemy's generator in this lane has been destroyed (to spawn UM) */
	bool bEnemyGeneratorDestroyed;

	/* amount of time to wait inbetween spawning minions in a wave */
	UPROPERTY(EditDefaultsOnly, Category = Spawner)
	float minionSpawnTime;

	/* amount of time inbetwen waves */
	UPROPERTY(EditDefaultsOnly, Category = Spawner)
	float waveTime;

	/* counter used for the wave spawner to to track which minion its spawning */
	int32 waveCounter;

	/* what lane this is a spawner for */
	int32 lane;

	/* whhat team this is a spawner for */
	UPROPERTY(EditAnywhere, Category = Spawner)
	int32 teamIndex;

	/* array of minion types to spawn for a normal wave */
	UPROPERTY(EditAnywhere, Category = Spawner)
	TArray<TSubclassOf<AMinionCharacter> > normalWave;

	/* array of minion types to spawn for a Ultra wave */
	UPROPERTY(EditAnywhere, Category = Spawner)
	TArray<TSubclassOf<AMinionCharacter> > ultraWave;

	/* where to spawn the minions for this lane */
	UPROPERTY(EditAnywhere, Category = Spawner)
	ATargetPoint* spawnLocation;

	/* reference to the enabler for this team */
	ARealmEnabler* teamEnabler;

public:

	/* each objective in the lane */
	UPROPERTY(EditAnywhere, Category = TeamBase)
	TArray<ARealmObjective*> laneObjectives;

	/* reference to the enemy's lane manager of the same lane */
	UPROPERTY(EditAnywhere, Category = Spawner)
	ALaneManager* enemyLane;

	/* called when the game mode starts the match */
	void MatchStarted();

	/* start the spawn of minions */
	void StartMinionWaveSpawning();

	/* spawn a wave of minions and then wait a time interval */
	void StartSpawningWave();

	/* spawn the next individual minion in the wave then check to see if the wave is finished */
	void SpawnNextMinion();

	/* stop spawning minion waves (called when the game ends usually) */
	void StopMinionWaveSpawning();

	/* called when an enemy generator is destroyed */
	void EnemyLaneGeneratorDestroyed();

	/* called when an enemy generator is repaired */
	void EnemyLaneGeneratorRepaired();

	/* gets the current objective for the specified lane */
	UFUNCTION(BlueprintCallable, Category = TeamBase)
	ARealmObjective* GetCurrentLaneObjective() const;

	/* gets the team's enabler */
	UFUNCTION(BlueprintCallable, Category = TeamBase)
	ARealmEnabler* GetTeamEnabler() const;

	/* gets the enemy lane manager */
	UFUNCTION(BlueprintCallable, Category = TeamBase)
	ALaneManager* GetEnemyLaneManager() const;
};