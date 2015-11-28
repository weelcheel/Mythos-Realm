#include "Realm.h"
#include "LaneManager.h"
#include "RealmLaneMinionAI.h"
#include "RealmObjective.h"
#include "MinionCharacter.h"
#include "RealmEnabler.h"

ALaneManager::ALaneManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	bEnemyGeneratorDestroyed = false;

	minionSpawnTime = 0.87f;
	waveTime = 55.f;
}

void ALaneManager::MatchStarted()
{
	if (Role == ROLE_Authority)
		GetWorld()->GetTimerManager().SetTimer(waveTimer, this, &ALaneManager::StartMinionWaveSpawning, 5.f);
}

void ALaneManager::StartMinionWaveSpawning()
{
	StartSpawningWave();
	GetWorld()->GetTimerManager().SetTimer(minionTimer2, this, &ALaneManager::StartSpawningWave, waveTime, true);
}

void ALaneManager::StartSpawningWave()
{
	waveCounter = 0;
	SpawnNextMinion();
}

void ALaneManager::SpawnNextMinion()
{
	if ((!bEnemyGeneratorDestroyed && waveCounter < normalWave.Num()) || (bEnemyGeneratorDestroyed && waveCounter < ultraWave.Num()))
	{
		AMinionCharacter* minion;
		if (bEnemyGeneratorDestroyed)
			minion = GetWorld()->SpawnActor<AMinionCharacter>(ultraWave[waveCounter], spawnLocation->GetActorLocation(), FRotator::ZeroRotator);
		else
			minion = GetWorld()->SpawnActor<AMinionCharacter>(normalWave[waveCounter], spawnLocation->GetActorLocation(), FRotator::ZeroRotator);

		if (minion)
		{
			minion->SetTeamIndex(teamIndex);
			ARealmLaneMinionAI* minionAI = GetWorld()->SpawnActor<ARealmLaneMinionAI>(minion->GetActorLocation(), minion->GetActorRotation());
			minionAI->SetLaneManager(this);
			minionAI->Possess(minion);
		}

		waveCounter++;
		GetWorld()->GetTimerManager().SetTimer(minionTimer, this, &ALaneManager::SpawnNextMinion, minionSpawnTime, false);
	}
}

void ALaneManager::StopMinionWaveSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(waveTimer);
	GetWorld()->GetTimerManager().ClearTimer(minionTimer);
	GetWorld()->GetTimerManager().ClearTimer(minionTimer2);
}

void ALaneManager::EnemyLaneGeneratorDestroyed()
{
	bEnemyGeneratorDestroyed = true;
}

void ALaneManager::EnemyLaneGeneratorRepaired()
{
	bEnemyGeneratorDestroyed = false;
}

ARealmObjective* ALaneManager::GetCurrentLaneObjective() const
{
	for (ARealmObjective* obj : laneObjectives)
	{
		if (IsValid(obj) && obj->IsAlive())
			return obj;
	}
	
	return teamEnabler;
}

ARealmEnabler* ALaneManager::GetTeamEnabler() const
{
	return teamEnabler;
}

ALaneManager* ALaneManager::GetEnemyLaneManager() const
{
	return enemyLane;
}