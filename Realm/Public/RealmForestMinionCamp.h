#pragma once

#include "RealmForestMinionCamp.generated.h"

class AMinionCharacter;

UCLASS()
class AForestCamp : public AActor
{
	GENERATED_UCLASS_BODY()

	/* what minion we need to spawn */
	UPROPERTY(EditAnywhere, Category = MinionType)
	TArray<TSubclassOf<AMinionCharacter> > minionTypes;

	/* where we need to spawn these minions */
	UPROPERTY(EditAnywhere, Category = MinionSpawns)
	TArray<ATargetPoint*> spawnPoints;

	/* how much time it takes to spawn these minions after they all die */
	UPROPERTY(EditAnywhere, Category = MinionSpawns)
	float spawnTime;

	/* spawn timer */
	FTimerHandle spawnTimer;

	/* death count */
	int32 deathCount = 0;

	/* already spawned minions this life cycle */
	bool bAlreadySpawned = false;

public:

	/* called when a spawned camp memeber dies */
	UFUNCTION(BlueprintCallable, Category=Minion)
	void SpawnedCharacterDied();

	/* called when to spawn minions */
	void SpawnMinions();
};