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

	objectives.Enqueue(laneManager->laneObjectives[0]);
	for (int32 i = 0; i < laneManager->enemyLane->laneObjectives.Num(); i++)
		objectives.Enqueue(laneManager->enemyLane->laneObjectives[i]);

	objectives.Dequeue(objectiveTarget);
	MoveToActor(objectiveTarget);
	currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

	minionCharacter = mc;

	FTimerHandle t;
	GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmLaneMinionAI::ReevaluateTargets, 0.8f, true);
	GetWorldTimerManager().SetTimer(t, this, &ARealmLaneMinionAI::CheckReachedObjective, 0.8f, true);
}

void ARealmLaneMinionAI::OnTargetEnterRadius(class APawn* pawn)
{
	AGameCharacter* gc = Cast<AGameCharacter>(pawn);
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (!IsValid(gc))
		return;

	if (gc->GetTeamIndex() == mc->GetTeamIndex() || !gc->IsAlive())
		return;

	if (currentTargetPriority == ELaneMinionTargetPriority::LMTP_ObjectiveTarget)
	{
		ELaneMinionTargetPriority priority;

		gc->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (gc->IsA(ATurret::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		SetNewTarget(gc, priority);
		mc->StartAutoAttack();
	}
}

void ARealmLaneMinionAI::CheckReachedObjective()
{
	if (!IsValid(objectiveTarget))
		return;

	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());
	if (!IsValid(mc))
		return;

	if (currentTargetPriority == ELaneMinionTargetPriority::LMTP_ObjectiveTarget)
	{
		float dist = (objectiveTarget->GetActorLocation() - mc->GetActorLocation()).Size2D();
		if (dist < 350.f && (objectiveTarget->GetTeamIndex() == mc->GetTeamIndex() || !objectiveTarget->IsAlive()))
		{
			objectives.Dequeue(objectiveTarget);
			MoveToActor(objectiveTarget);
			currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
		}
	}
}

void ARealmLaneMinionAI::ReevaluateTargets()
{
	if (!IsValid(minionCharacter))
		return;

	if (IsValid(minionCharacter->GetCurrentTarget()))
	{
		float distsq = (minionCharacter->GetCurrentTarget()->GetActorLocation() - GetCharacter()->GetActorLocation()).SizeSquared2D();
		if (!minionCharacter->GetCurrentTarget()->IsAlive() || distsq > FMath::Square(minionCharacter->GetCurrentValueForStat(EStat::ES_AARange)))
			NeedsNewCommand();
		else
			minionCharacter->StartAutoAttack();
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
	if (!IsValid(minionCharacter))
		return;

	float leastDistance = -1.f;
	int32 leastInd = -1;

	inRangeTargets.Empty();
	for (TActorIterator<AGameCharacter> chritr(GetWorld()); chritr; ++chritr)
	{
		AGameCharacter* gc = *chritr;
		if (IsValid(gc) && gc->IsAlive() && gc != minionCharacter && gc->GetTeamIndex() != minionCharacter->GetTeamIndex())
		{
			float dist = (gc->GetActorLocation() - minionCharacter->GetActorLocation()).Size2D();
			if (dist <= minionCharacter->GetCurrentValueForStat(EStat::ES_AARange))
				inRangeTargets.AddUnique(gc);
		}
	}

	for (int32 i = 0; i < inRangeTargets.Num(); i++)
	{
		if (!IsValid(inRangeTargets[i]))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (!inRangeTargets[i]->IsA(AMinionCharacter::StaticClass()))
			continue;

		float distanceSq = (inRangeTargets[i]->GetActorLocation() - minionCharacter->GetActorLocation()).SizeSquared2D();
		if (!inRangeTargets[i]->IsAlive() || distanceSq > FMath::Square(710.f))
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

		minionCharacter->StartAutoAttack();
	}
	else
	{
		ARealmObjective* oc = laneManager->GetEnemyLaneManager()->GetCurrentLaneObjective();
		MoveToActor(objectiveTarget);
		currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		minionCharacter->StopAutoAttack();
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

void ARealmLaneMinionAI::CharacterInAttackRange()
{
	if (!IsValid(minionCharacter))
		return;

	//see if there are any friendlies in range
	for (TActorIterator<AMinionCharacter> minion(GetWorld()); minion; ++minion)
	{
		AMinionCharacter* mc = *minion;
		FVector targetVector = mc->GetActorLocation() - minionCharacter->GetActorLocation();
		float distance = targetVector.Size();
		if (IsValid(mc) && mc->IsAlive() && mc != minionCharacter && mc->GetTeamIndex() == minionCharacter->GetTeamIndex() && distance <= 50.f)
		{
			FTimerHandle handle;
			GetWorldTimerManager().SetTimer(handle, this, &ARealmLaneMinionAI::CharacterInAttackRange, 0.15f);
			bRepositioned = true;

			FVector newLoc = targetVector.RotateAngleAxis(FMath::RandRange(90, 180), FVector(0.f, 0.f, 1.f));
			minionCharacter->StopAutoAttack();

			MoveToLocation(minionCharacter->GetActorLocation() + (newLoc.Rotation().Vector() * 50.f));

			return;
		}
	}

	if (bRepositioned)
	{
		bRepositioned = false;
		ReevaluateTargets();
		minionCharacter->StartAutoAttack();
	}
}

void ARealmLaneMinionAI::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::Destroy(bNetForce, bShouldModifyLevel);
}