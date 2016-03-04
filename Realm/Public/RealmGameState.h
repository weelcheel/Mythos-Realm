#pragma once

#include "GameFramework/GameState.h"
#include "RealmGameState.generated.h"

struct FRealmChatEntry;

UCLASS()
class ARealmGameState : public AGameState
{
	friend class ARealmGameMode;

	GENERATED_UCLASS_BODY()

protected:

	/* the time when this match was started */
	UPROPERTY(replicated)
	float matchStartTime;

	/* array of ints keeping count of how many kills (or score) each team has */
	UPROPERTY(replicated)
	TArray<int32> teamScores;

	/* array of chat entries for this game */
	UPROPERTY()
	TArray<FRealmChatEntry> gameChat;

public:

	/** broadcast death for objective to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastObjectiveDeath(APawn* killerPawn, ARealmObjective* objectiveDestroyed);

	/* broadcast chat from a source and then notify each player */
	UFUNCTION(reliable, NetMulticast, BlueprintCallable, Category = Chat)
	void BroadcastChat(const FRealmChatEntry& broadcastChat);

	/* gets the amount of time that has passed between now and when the match started */
	UFUNCTION(BlueprintCallable, Category = GameTime)
	float GetMatchTime() const;

	/* gets the score for the specified team */
	UFUNCTION(BlueprintCallable, Category = Score)
	int32 GetTeamScore(int32 index) const;
};