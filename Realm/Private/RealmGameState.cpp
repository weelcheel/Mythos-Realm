#include "Realm.h"
#include "RealmGameState.h"

ARealmGameState::ARealmGameState(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	matchStartTime = -1.f;
}

void ARealmGameState::BroadcastObjectiveDeath_Implementation(APawn* killerPawn, ARealmObjective* objectiveDestroyed)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		ARealmPlayerController* TestPC = Cast<ARealmPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnObjectiveDeathMessage(killerPawn, objectiveDestroyed);
		}
	}
}

float ARealmGameState::GetMatchTime() const
{
	if (matchStartTime >= 0.f)
		return GetWorld()->GetTimeSeconds() - matchStartTime;
	else
		return 0.f;
}

int32 ARealmGameState::GetTeamScore(int32 index) const
{
	if (index >= 0 && index < teamScores.Num())
		return teamScores[index];
	else
		return -1;
}

void ARealmGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmGameState, matchStartTime);
	DOREPLIFETIME(ARealmGameState, teamScores);
}