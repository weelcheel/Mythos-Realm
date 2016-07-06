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
	aggroDistance = 420.f;
	targetRadius->SightRadius = aggroDistance;
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
	//GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmLaneMinionAI::ReevaluateTargets, 0.25f, true);
	//GetWorldTimerManager().SetTimer(t, this, &ARealmLaneMinionAI::CheckReachedObjective, 0.1f, true);
}

void ARealmLaneMinionAI::OnTargetEnterRadius(class APawn* pawn)
{
	/*AGameCharacter* gc = Cast<AGameCharacter>(pawn);
	AMinionCharacter* mc = Cast<AMinionCharacter>(GetCharacter());

	if (!IsValid(gc))
		return;

	if (gc->GetTeamIndex() == mc->GetTeamIndex() || !gc->IsAlive())
		return;

	if (currentTargetPriority == ELaneMinionTargetPriority::LMTP_ObjectiveTarget)
	{
		ELaneMinionTargetPriority priority;

		gc->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (gc->IsA(ARealmObjective::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		SetNewTarget(gc, priority);
		mc->StartAutoAttack();
	}*/

	AGameCharacter* gc = Cast<AGameCharacter>(pawn);

	if (!IsValid(gc) || !IsValid(minionCharacter))
		return;

	//check for objective reaching
	if (gc->IsA(ARealmObjective::StaticClass()))
	{
		if (IsValid(objectiveTarget) && objectiveTarget == gc)
		{
			objectives.Dequeue(objectiveTarget);
			if (!IsValid(minionCharacter->GetCurrentTarget()) && gc->GetTeamIndex() == minionCharacter->GetTeamIndex())
			{
				MoveToActor(objectiveTarget);
				return;
			}
		}
	}

	if (gc->GetTeamIndex() == minionCharacter->GetTeamIndex() || !gc->IsAlive())
		return;

	if (!IsValid(minionCharacter->GetCurrentTarget()))
	{
		ELaneMinionTargetPriority priority;

		gc->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (gc->IsA(ARealmObjective::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		SetNewTarget(gc, priority);
		minionCharacter->StartAutoAttack();

		GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmLaneMinionAI::ReevaluateTargets, 0.33f, true);
	}
}

void ARealmLaneMinionAI::CheckReachedObjective()
{
	if (!IsValid(objectiveTarget))
		return;

	if (!IsValid(minionCharacter->GetCurrentTarget()))
	{
		float dist = (objectiveTarget->GetActorLocation() - minionCharacter->GetActorLocation()).Size2D();
		if (dist < 450.f && (objectiveTarget->GetTeamIndex() == minionCharacter->GetTeamIndex() || !objectiveTarget->IsAlive()))
		{
			objectives.Dequeue(objectiveTarget);

			if (!IsValid(minionCharacter->GetCurrentTarget()))
			{
				MoveToActor(objectiveTarget);
				currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
			}
		}
	}
}

void ARealmLaneMinionAI::ReevaluateTargets()
{
	if (!IsValid(minionCharacter))
		return;

	if (IsValid(minionCharacter->GetCurrentTarget()))
	{
		float distsq = (minionCharacter->GetActorLocation() - minionCharacter->GetCurrentTarget()->GetActorLocation()).SizeSquared2D();
		if (!minionCharacter->GetCurrentTarget()->IsAlive() || !minionCharacter->CanSeeOtherCharacter(minionCharacter->GetCurrentTarget()) || !minionCharacter->GetCurrentTarget()->IsTargetable() || distsq > FMath::Square(aggroDistance / 2.f))
			NeedsNewCommand();
		else if (distsq > FMath::Square(minionCharacter->GetCurrentValueForStat(EStat::ES_AARange)))
			MoveToActor(minionCharacter->GetCurrentTarget(), minionCharacter->GetCurrentValueForStat(EStat::ES_AARange));
		else
			minionCharacter->StartAutoAttack();
	}
	else
		NeedsNewCommand();
}

void ARealmLaneMinionAI::SetNewTarget(AGameCharacter* newTarget, ELaneMinionTargetPriority targetPriority)
{
	if (IsValid(newTarget))
	{
		minionCharacter->SetCurrentTarget(newTarget);
		currentTargetPriority = targetPriority;
		//inRangeTargets.Remove(newTarget);
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

	if (nextTarget && nextTargetPriority >= currentTargetPriority)
	{
		SetNewTarget(nextTarget, nextTargetPriority);
		nextTarget = nullptr;
		nextTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		minionCharacter->StartAutoAttack();

		return;
	}

	TArray<FHitResult> hits;
	FVector start = minionCharacter->GetActorLocation();
	FVector end = start;
	end.Z += 5.f;

	TArray<AGameCharacter*> possibleTargets;

	GetWorld()->SweepMultiByChannel(hits, start, end, minionCharacter->GetActorRotation().Quaternion(), ECC_Visibility, FCollisionShape::MakeSphere(aggroDistance));
	for (FHitResult hit : hits)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
		if (IsValid(gc) && gc->IsAlive() && minionCharacter->GetTeamIndex() != gc->GetTeamIndex())
			possibleTargets.AddUnique(gc);
	}

	//first aggro any minions first
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(AMinionCharacter::StaticClass()))
		{
			ELaneMinionTargetPriority priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
			SetNewTarget(gc, priority);
			minionCharacter->StartAutoAttack();

			return;
		}
	}

	//then aggro an objective if we can 
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(ARealmObjective::StaticClass()))
		{
			ELaneMinionTargetPriority priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
			SetNewTarget(gc, priority);
			minionCharacter->StartAutoAttack();

			return;
		}
	}

	//lastly aggro any mythos
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(APlayerCharacter::StaticClass()))
		{
			ELaneMinionTargetPriority priority = ELaneMinionTargetPriority::LMTP_ClosestMythos;
			SetNewTarget(gc, priority);
			minionCharacter->StartAutoAttack();

			return;
		}
	}

	//no in-range targets, so travel to the next objective target
	MoveToActor(objectiveTarget);
	currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;
	minionCharacter->StopAutoAttack();

	GetWorldTimerManager().ClearTimer(rangeTimer);

	/*float leastDistance = -1.f;
	int32 leastInd = -1;

	TArray<AGameCharacter*> irt;
	for (TActorIterator<AGameCharacter> chritr(GetWorld()); chritr; ++chritr)
	{
		AGameCharacter* gc = *chritr;

		if (IsValid(gc) && gc->IsAlive() && gc != minionCharacter && gc->GetTeamIndex() != minionCharacter->GetTeamIndex())
		{
			float dist = (gc->GetActorLocation() - minionCharacter->GetActorLocation()).Size2D();
			if (dist <= minionCharacter->sightRadius / 3.f && minionCharacter->CanSeeOtherCharacter(gc))
				irt.AddUnique(gc);
		}
	}

	for (int32 i = 0; i < irt.Num(); i++)
	{
		if (!IsValid(irt[i]))
		{
			irt.RemoveAt(i);
			continue;
		}

		if (!irt[i]->IsA(AMinionCharacter::StaticClass()))
			continue;

		float distanceSq = (irt[i]->GetActorLocation() - minionCharacter->GetActorLocation()).SizeSquared2D();
		if (!irt[i]->IsAlive() || distanceSq > FMath::Square(minionCharacter->sightRadius))
		{
			irt.RemoveAt(i);
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
		irt[leastInd]->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
		if (irt[leastInd]->IsA(ATurret::StaticClass()))
			priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

		AGameCharacter* test = irt[leastInd];
		SetNewTarget(test, priority);

		minionCharacter->SetCurrentTarget(test);
		currentTargetPriority = priority;

		minionCharacter->StartAutoAttack();
	}
	else
	{
		leastDistance = -1.f;
		leastInd = -1;

		for (int32 i = 0; i < inRangeTargets.Num(); i++)
		{
			float distanceSq = (irt[i]->GetActorLocation() - minionCharacter->GetActorLocation()).SizeSquared2D();

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
			irt[leastInd]->IsA(APlayerCharacter::StaticClass()) ? priority = ELaneMinionTargetPriority::LMTP_ClosestMythos : priority = ELaneMinionTargetPriority::LMTP_ClosestMinion;
			if (irt[leastInd]->IsA(ATurret::StaticClass()))
				priority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

			SetNewTarget(irt[leastInd], priority);

			minionCharacter->StartAutoAttack();
		}
		else
		{
			MoveToActor(objectiveTarget);
			currentTargetPriority = ELaneMinionTargetPriority::LMTP_ObjectiveTarget;

			minionCharacter->StopAutoAttack();
		}
	}*/
}

void ARealmLaneMinionAI::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{
	if (!IsValid(distressedUnit) || !IsValid(enemyTarget))
		return;

	if (distressedUnit->IsA(APlayerCharacter::StaticClass()) && enemyTarget->IsA(APlayerCharacter::StaticClass()))
	{
		nextTarget = enemyTarget;
		nextTargetPriority = ELaneMinionTargetPriority::LMTP_DCMythosMythos;
		NeedsNewCommand();
	}
}

void ARealmLaneMinionAI::CharacterInAttackRange()
{
	if (!IsValid(minionCharacter))
		return;

	/*FVector newLoc = minionCharacter->GetActorLocation();
	TArray<FHitResult> hits;
	FVector end = newLoc;
	end.Z += 5.f;

	bool bShouldReposition = false;
	GetWorld()->SweepMultiByChannel(hits, newLoc, end, minionCharacter->GetActorRotation().Quaternion(), ECC_Pawn, FCollisionShape::MakeSphere(75.f));
	for (FHitResult hit : hits)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
		if (IsValid(gc) && gc->IsA(AMinionCharacter::StaticClass()) && gc != minionCharacter && gc->IsAlive() && gc->GetTeamIndex() == minionCharacter->GetTeamIndex())
			bShouldReposition = true;
	}
	
	if (bShouldReposition)
	{
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &ARealmLaneMinionAI::CharacterInAttackRange, 0.15f);
		bRepositioned = true;

		for (int32 i = 1; i < 360; i++) //rotate around the target location by degree until we find an open spot to go
		{
			bool bDirection = FMath::RandBool();
			newLoc = minionCharacter->GetActorLocation().RotateAngleAxis(bDirection ? i : i * -1, FVector(0.f, 0.f, 1.f));

			GetWorld()->SweepMultiByChannel(hits, newLoc, end, minionCharacter->GetActorRotation().Quaternion(), ECC_Pawn, FCollisionShape::MakeSphere(75.f));

			bool bFoundCharacter = false;
			for (FHitResult hit : hits)
			{
				AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
				if (IsValid(gc) && gc->IsA(AMinionCharacter::StaticClass()) && gc != minionCharacter && gc->IsAlive() && gc->GetTeamIndex() == minionCharacter->GetTeamIndex())
					bFoundCharacter = true;
			}

			if (!bFoundCharacter)
				break;
		}

		minionCharacter->StopAutoAttack();
		MoveToLocation(newLoc);

		return;
	}*/

	//see if there are any friendlies in range
	for (TActorIterator<AMinionCharacter> minion(GetWorld()); minion; ++minion)
	{
		AMinionCharacter* mc = *minion;
		FVector targetVector = mc->GetActorLocation() - minionCharacter->GetActorLocation();
		float distance = targetVector.Size();
		if (IsValid(mc) && mc->IsAlive() && mc != minionCharacter && mc->GetTeamIndex() == minionCharacter->GetTeamIndex() && distance <= 75.f)
		{
			FTimerHandle handle;
			GetWorldTimerManager().SetTimer(handle, this, &ARealmLaneMinionAI::CharacterInAttackRange, 0.15f);
			bRepositioned = true;

			FVector newLoc = targetVector.RotateAngleAxis(FMath::RandRange(90.f, 180.f), FVector(0.f, 0.f, 1.f));

			repositionTarget = minionCharacter->GetCurrentTarget();
			minionCharacter->StopAutoAttack();
			MoveToLocation(minionCharacter->GetActorLocation() + (newLoc.Rotation().Vector() * 25.f));

			GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmLaneMinionAI::ReevaluateTargets, 0.8f, true);

			return;
		}
	}

	if (bRepositioned)
	{
		bRepositioned = false;
		minionCharacter->SetCurrentTarget(repositionTarget);
		minionCharacter->StartAutoAttack();
	}
}

void ARealmLaneMinionAI::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::Destroy(bNetForce, bShouldModifyLevel);
}