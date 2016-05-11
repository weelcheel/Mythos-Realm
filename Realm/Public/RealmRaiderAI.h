#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmRaiderAI.generated.h"

class ARaiderCharacter;
class AGameCharacter;
class ARealmObjective;

UCLASS()
class ARealmRaiderAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the character casted to a raider character */
	UPROPERTY()
	ARaiderCharacter* raiderCharacter;

	/* current objective that this unit is trying to move to */
	ARealmObjective* objectiveTarget;

	/* current enemy unit this target is attacking (if any) */
	AGameCharacter* enemyTarget;

	/* queue of objectives we need to visit to keep pathing in lane*/
	TQueue<ARealmObjective*> objectives;

	/* timer for range checking */
	FTimerHandle rangeTimer, spawnIntroTimer;

	/* location that this raider spawned */
	FVector spawnLocation;

	/* target out of aggro range */
	void ReevaluateTargets();

	/* check for reached objectives */
	void CheckReachedObjective();

public:

	/* lane manager that controls this minion */
	UPROPERTY(VisibleAnywhere, Category = Lane)
	ALaneManager* laneManager;

	virtual void Possess(APawn* InPawn) override;

	virtual void NeedsNewCommand() override;

	/* called when the possessed character dies */
	void OnDeath();
};