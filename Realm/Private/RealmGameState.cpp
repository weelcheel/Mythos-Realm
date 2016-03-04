#include "Realm.h"
#include "RealmGameState.h"
#include "Chat.h"
#include "RealmPlayerState.h"

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

void ARealmGameState::BroadcastChat_Implementation(const FRealmChatEntry& broadcastChat)
{
	gameChat.Add(broadcastChat);
	
	if (Role < ROLE_Authority || (Role == ROLE_Authority && GetNetMode() == NM_ListenServer))
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ARealmPlayerController* TestPC = Cast<ARealmPlayerController>(*It);
			ARealmPlayerState* ps = Cast<ARealmPlayerState>(TestPC->PlayerState);
			if (broadcastChat.bAllChat)
			{
				if (IsValid(TestPC) && TestPC->IsLocalController())
					TestPC->ClientReceiveChat(broadcastChat);
			}
			else if (IsValid(ps))
			{
				if (IsValid(TestPC) && TestPC->IsLocalController() && ps->GetTeamIndex() == broadcastChat.senderTeamIndex)
					TestPC->ClientReceiveChat(broadcastChat);
			}
		}
	}
}

void ARealmGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmGameState, matchStartTime);
	DOREPLIFETIME(ARealmGameState, teamScores);
}