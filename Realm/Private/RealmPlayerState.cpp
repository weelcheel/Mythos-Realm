#include "Realm.h"
#include "RealmPlayerState.h"
#include "UnrealNetwork.h"

ARealmPlayerState::ARealmPlayerState(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	teamIndex = -1;
}

int32 ARealmPlayerState::GetTeamIndex() const
{
	return teamIndex;
}

void ARealmPlayerState::SetTeamIndex(int32 newTeam)
{
	teamIndex = newTeam;
}

void ARealmPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmPlayerState, teamIndex);
	DOREPLIFETIME(ARealmPlayerState, chosenCharacterClass);
	DOREPLIFETIME(ARealmPlayerState, teamPlayerIndex);
}