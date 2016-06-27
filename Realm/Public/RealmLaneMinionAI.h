#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmLaneMinionAI.generated.h"

class ALaneManager;
class AGameCharacter;
class AMinionCharacter;
class ARealmObjective;

UENUM()
enum class ELaneMinionTargetPriority : uint8
{
	LMTP_ObjectiveTarget,
	LMTP_ClosestMythos,
	LMTP_ClosestMinion,
	LMTP_DCMythosMinion,	//mythos attacking minion
	LMTP_DCTurretMinion,	//turret attacking minion
	LMTP_DCMinionMinion,	//minion attacking minion
	LMTP_DCMinionMythos,	//minion attacking mythos
	LMTP_DCMythosMythos,	//mythos attacking mythos
	LMTP_MAX
};


UCLASS()
class ARealmLaneMinionAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the character casted to a minion character */
	UPROPERTY()
	AMinionCharacter* minionCharacter;

	/* lane manager that controls this minion */
	UPROPERTY(VisibleAnywhere, Category = Lane)
	ALaneManager* laneManager;

	/* timer for range checking */
	UPROPERTY()
	FTimerHandle rangeTimer;

	/* current target priority */
	UPROPERTY()
	ELaneMinionTargetPriority currentTargetPriority;

	/* next target */
	UPROPERTY()
	AGameCharacter* nextTarget;

	/* target we attack after repositioning */
	UPROPERTY()
	AGameCharacter* repositionTarget;

	/* objective target */
	UPROPERTY()
	ARealmObjective* objectiveTarget;

	/* next target priority */
	UPROPERTY()
	ELaneMinionTargetPriority nextTargetPriority;

	UPROPERTY()
	bool bRepositioned;

	/* what distance this minion will keep aggro for */
	UPROPERTY(EditDefaultsOnly, Category = Aggro)
	float aggroDistance;

	/* queue of objectives we need to visit to keep pathing in lane*/
	TQueue<ARealmObjective*> objectives;

	/* target out of aggro range */
	void ReevaluateTargets();

	/* check for reached objectives */
	void CheckReachedObjective();

	/* called whenever an enemy walks into attack radius */
	UFUNCTION()
	virtual void OnTargetEnterRadius(class APawn* seenPawn) override;

	/* set a new target for this minion */
	void SetNewTarget(AGameCharacter* newTarget, ELaneMinionTargetPriority targetPriority);

public:

	virtual void Possess(APawn* InPawn) override;

	void SetLaneManager(ALaneManager* newLaneManager);
	virtual void NeedsNewCommand() override;

	/* called when the controller receives a call for help from another unit */
	void ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget);

	virtual void Destroy(bool bNetForce /* = false */, bool bShouldModifyLevel /* = true */);

	virtual void CharacterInAttackRange() override;
};