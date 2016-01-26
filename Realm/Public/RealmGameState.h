#pragma once

#include "GameFramework/GameState.h"
#include "RealmGameState.generated.h"

UCLASS()
class ARealmGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

public:

	/** broadcast death for objective to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastObjectiveDeath(APawn* killerPawn, ARealmObjective* objectiveDestroyed);
};