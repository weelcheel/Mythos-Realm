#include "Realm.h"
#include "RealmLaneMinionAI.h"
#include "MinionCharacter.h"
#include "LaneManager.h"
#include "RealmObjective.h"
#include "PlayerCharacter.h"
#include "RealmTurret.h"

ARealmLaneMinionAI::ARealmLaneMinionAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void ARealmLaneMinionAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	AMinionCharacter* mc = Cast<AMinionCharacter>(InPawn);
	if (mc)
		mc->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	mc->GetStatsManager()->SetMaxHealth();

	ARealmObjective* oc = laneManager->GetEnemyLaneManager()->GetCurrentLaneObjective();
	MoveToActor(oc, mc->GetAutoAttackManager()->GetCurrentAutoAttackRange());
	currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

	GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmLaneMinionAI::ReevaluateTargets, 1.8f, true);
}

void ARealmLaneMinionAI::OnTargetEnterRadius(class APawn* pawn)
{
	AGameCharacter* gc = Cast<AGameCharacter>(pawn);
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (!IsValid(gc))
		return;

	if (gc->GetTeamIndex() == mc->GetTeamIndex() || !gc->IsAlive())
		return;

	if (currentTargetPriority > ELaneMinionTargetPriority::LMTP_ObjectiveTarget)
		inRangeTargets.AddUnique(gc);
	else
	{
		ELaneMinionTargetPriority priority;

		gc->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (gc->IsA(ATurret::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		SetNewTarget(gc, priority);
		mc->StartAutoAttack();
	}
}

void ARealmLaneMinionAI::ReevaluateTargets()
{
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (!IsValid(mc))
		return;

	if (IsValid(mc->GetCurrentTarget()))
	{
		float distsq = (mc->GetCurrentTarget()->GetActorLocation() - GetCharacter()->GetActorLocation()).SizeSquared2D();
		if (!mc->GetCurrentTarget()->IsAlive() || distsq > FMath::Square(targetRadius->SightRadius / 3.f))
			NeedsNewCommand();
		else
			mc->StartAutoAttack();
	}
	else
		NeedsNewCommand();

	if (nextTarget && nextTargetPriority >= currentTargetPriority)
	{
		SetNewTarget(nextTarget, nextTargetPriority);
		nextTarget = nullptr;
		nextTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
	}
}

void ARealmLaneMinionAI::SetNewTarget(AGameCharacter* newTarget, ELaneMinionTargetPriority targetPriority)
{
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (IsValid(mc) && IsValid(newTarget))
	{
		mc->SetCurrentTarget(newTarget);
		currentTargetPriority = targetPriority;
		inRangeTargets.Remove(newTarget);
	}
}

void ARealmLaneMinionAI::SetLaneManager(ALaneManager* newLaneManager)
{
	laneManager = newLaneManager;
}

void ARealmLaneMinionAI::NeedsNewCommand()
{
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (!IsValid(mc))
		return;

	float leastDistance = -1.f;
	int32 leastInd = -1;

	for (int32 i = 0; i < inRangeTargets.Num(); i++)
	{
		if (!IsValid(inRangeTargets[i]) || !inRangeTargets[i]->IsAlive())
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		float distanceSq = (inRangeTargets[i]->GetActorLocation() - mc->GetActorLocation()).SizeSquared2D();
		if (distanceSq > FMath::Square(targetRadius->SightRadius))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (leastDistance == -1.f)
		{
			leastDistance = distanceSq;
			leastInd = i;
			continue;
		}

		if (distanceSq < leastDistance)
		{
			leastDistance = distanceSq;
			leastInd = i;
		}
	}

	if (leastInd >= 0)
	{
		ELaneMinionTargetPriority priority;
		inRangeTargets[leastInd]->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (inRangeTargets[leastInd]->IsA(ATurret::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		SetNewTarget(inRangeTargets[leastInd], priority);
	}
	else
	{
		mc->StopAutoAttack();
		ARealmObjective* oc = laneManager->GetEnemyLaneManager()->GetCurrentLaneObjective();
		MoveToActor(oc, mc->GetAutoAttackManager()->GetCurrentAutoAttackRange());
		currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
	}
}

void ARealmLaneMinionAI::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{
	if (!IsValid(distressedUnit) || !IsValid(enemyTarget))
		return;

	ELaneMinionTargetPriority thisPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

	if (distressedUnit->IsA(APlayerCharacter::StaticClass()) && enemyTarget->IsA(APlayerCharacter::StaticClass()))
		thisPriority = ELaneMinionTargetPriority::LMTP_DCMythosMythos;
	else if (distressedUnit->IsA(APlayerCharacter::StaticClass()) && enemyTarget->IsA(AMinionCharacter::StaticClass()))
		thisPriority = ELaneMinionTargetPriority::LMTP_DCMinionMythos;
	else if (distressedUnit->IsA(AMinionCharacter::StaticClass()) && enemyTarget->IsA(AMinionCharacter::StaticClass()))
		thisPriority = ELaneMinionTargetPriority::LMTP_DCMinionMinion;
	else if (distressedUnit->IsA(AMinionCharacter::StaticClass()) && enemyTarget->IsA(ATurret::StaticClass()))
		thisPriority = ELaneMinionTargetPriority::LMTP_DCTurretMinion;
	else if (distressedUnit->IsA(AMinionCharacter::StaticClass()) && enemyTarget->IsA(APlayerCharacter::StaticClass()))
		thisPriority = ELaneMinionTargetPriority::LMTP_DCMythosMinion;

	if (thisPriority >= nextTargetPriority)
	{
		nextTarget = enemyTarget;
		nextTargetPriority = thisPriority;
	}
}

void ARealmLaneMinionAI::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::Destroy(bNetForce, bShouldModifyLevel);
}