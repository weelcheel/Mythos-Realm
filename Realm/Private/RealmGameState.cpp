#include "Realm.h"
#include "RealmGameState.h"

ARealmGameState::ARealmGameState(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

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