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

void ARealmPlayerState::BroadcastDeath_Implementation(class ARealmPlayerState* KillerPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		ARealmPlayerController* TestPC = Cast<ARealmPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this);
		}
	}
}

void ARealmPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmPlayerState, teamIndex);
	DOREPLIFETIME(ARealmPlayerState, chosenCharacterClass);
	DOREPLIFETIME(ARealmPlayerState, teamPlayerIndex);
	DOREPLIFETIME(ARealmPlayerState, playerKills);
	DOREPLIFETIME(ARealmPlayerState, playerDeaths);
	DOREPLIFETIME(ARealmPlayerState, playerCreepScore);
	DOREPLIFETIME(ARealmPlayerState, playerTotalIncome);
	DOREPLIFETIME(ARealmPlayerState, playerAssists);
}