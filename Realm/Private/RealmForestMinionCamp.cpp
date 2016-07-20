#include "Realm.h"
#include "RealmForestMinionCamp.h"
#include "MinionCharacter.h"
#include "RealmForestMinionAI.h"

AForestCamp::AForestCamp(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void AForestCamp::SpawnedCharacterDied()
{
	deathCount++;

	if (deathCount >= minionTypes.Num())
	{
		deathCount = 0;
		GetWorldTimerManager().SetTimer(spawnTimer, this, &AForestCamp::SpawnMinions, spawnTime, false);
		bAlreadySpawned = false;
	}
}

void AForestCamp::SpawnMinions()
{
	if (Role < ROLE_Authority)
		return;

	if (minionTypes.Num() != spawnPoints.Num())
		return;

	if (bAlreadySpawned)
		return;

	for (int32 i = 0; i < minionTypes.Num(); i++)
	{
		AMinionCharacter* mc = GetWorld()->SpawnActor<AMinionCharacter>(minionTypes[i], spawnPoints[i]->GetActorLocation(), spawnPoints[i]->GetActorRotation());
		ARealmForestMinionAI* ai = GetWorld()->SpawnActor<ARealmForestMinionAI>();

		if (IsValid(mc) && IsValid(ai))
		{
			mc->SetTeamIndex(2);
			ai->Possess(mc);
			ai->homePosition = spawnPoints[i]->GetActorLocation();
			ai->campSpawner = this;
		}
	}

	bAlreadySpawned = true;
}