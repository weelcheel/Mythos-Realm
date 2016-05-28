#include "Realm.h"
#include "RealmRaiderAI.h"
#include "RealmRaider.h"

ARealmRaiderAI::ARealmRaiderAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void ARealmRaiderAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	ARaiderCharacter* mc = Cast<ARaiderCharacter>(InPawn);
	if (mc)
		mc->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	mc->GetStatsManager()->SetMaxHealth();

	FTimerHandle t;
	GetWorldTimerManager().SetTimer(rangeTimer, this, &ARealmRaiderAI::ReevaluateTargets, 0.8f, true);
	GetWorldTimerManager().SetTimer(t, this, &ARealmRaiderAI::CheckReachedObjective, 0.8f, true);
	GetWorldTimerManager().SetTimer(spawnIntroTimer, 7.f, false);

	spawnLocation = mc->GetActorLocation();

	mc->StartDespawnTimer();
}

void ARealmRaiderAI::CheckReachedObjective()
{
	if (GetWorldTimerManager().GetTimerRemaining(spawnIntroTimer) > 0.f)
		return;

	AGameCharacter* mc = Cast<AGameCharacter>(GetCharacter());
	if (!IsValid(mc))
		return;

	if (IsValid(mc->GetCurrentTarget()))
		return;

	if (mc->GetTeamIndex() == 3)
		return;
	else
	{
		if (IsValid(laneManager) && !IsValid(objectiveTarget))
		{
			objectives.Enqueue(laneManager->laneObjectives[0]);
			for (int32 i = 0; i < laneManager->enemyLane->laneObjectives.Num(); i++)
				objectives.Enqueue(laneManager->enemyLane->laneObjectives[i]);

			objectives.Dequeue(objectiveTarget);
			MoveToActor(objectiveTarget);
		}
	}

	if (!IsValid(objectiveTarget))
		return;

	float dist = (objectiveTarget->GetActorLocation() - mc->GetActorLocation()).Size2D();
	if (dist < 350.f && (objectiveTarget->GetTeamIndex() == mc->GetTeamIndex() || !objectiveTarget->IsAlive()))
	{
		objectives.Dequeue(objectiveTarget);

		if (!IsValid(mc->GetCurrentTarget()))
		{
			mc->SetCurrentTarget(objectiveTarget);
			mc->StartAutoAttack();
		}
	}
	else
		MoveToActor(objectiveTarget);
}

void ARealmRaiderAI::ReevaluateTargets()
{
	if (GetWorldTimerManager().GetTimerRemaining(spawnIntroTimer) > 0.f)
		return;

	AGameCharacter* mc = Cast<AGameCharacter>(GetCharacter());
	if (!IsValid(mc))
		return;

	if (IsValid(mc->GetCurrentTarget()))
	{
		float distsq = (mc->GetCurrentTarget()->GetActorLocation() - GetCharacter()->GetActorLocation()).SizeSquared2D();
		if (!mc->GetCurrentTarget()->IsAlive() || distsq > FMath::Square(mc->GetCurrentValueForStat(EStat::ES_AARange) * 1.5f))
		{
			NeedsNewCommand();
			return;
		}
		else
			mc->StartAutoAttack();
	}
	else
		NeedsNewCommand();
}

void ARealmRaiderAI::NeedsNewCommand()
{
	AGameCharacter* mc = Cast<AGameCharacter>(GetCharacter());
	if (!IsValid(mc))
		return;

	if (mc->GetTeamIndex() == 3)
	{
		AGameCharacter* closest = nullptr;
		float closestDist = -1.f;
		for (TActorIterator<AGameCharacter> chritr(GetWorld()); chritr; ++chritr)
		{
			AGameCharacter* gc = *chritr;
			if (gc->IsA(ARealmObjective::StaticClass()))
				continue;

			if (IsValid(gc) && gc->IsAlive() && gc != mc)
			{
				float dist = (gc->GetActorLocation() - mc->GetActorLocation()).Size2D();
				if (dist <= mc->GetCurrentValueForStat(EStat::ES_AARange) * 1.5f)
				{
					if (closestDist < 0.f || dist < closestDist)
					{
						closest = gc;
						closestDist = dist;
					}
				}
			}
		}

		if (IsValid(closest))
		{
			mc->SetCurrentTarget(closest);
			mc->StartAutoAttack();
		}
		else
		{
			mc->StopAutoAttack();
			MoveToLocation(spawnLocation);
		}
	}
	else
	{
		AGameCharacter* closest = nullptr;
		float closestDist = -1.f;
		for (TActorIterator<AGameCharacter> chritr(GetWorld()); chritr; ++chritr)
		{
			AGameCharacter* gc = *chritr;

			if (IsValid(gc) && gc->IsAlive() && gc != mc && gc->GetTeamIndex() != mc->GetTeamIndex())
			{
				float dist = (gc->GetActorLocation() - mc->GetActorLocation()).Size2D();
				if (dist <= mc->GetCurrentValueForStat(EStat::ES_AARange) * 1.5f)
				{
					if (closestDist < 0.f || dist < closestDist)
					{
						closest = gc;
						closestDist = dist;
					}
				}
			}
		}

		if (IsValid(closest))
		{
			mc->SetCurrentTarget(closest);
			mc->StartAutoAttack();
		}
		else if (IsValid(objectiveTarget))
		{
			mc->StopAutoAttack();
			MoveToActor(objectiveTarget);
		}
	}
}

void ARealmRaiderAI::OnDeath()
{
	AGameCharacter* mc = Cast<AGameCharacter>(GetCharacter());
	if (!IsValid(mc))
		return;

	if (IsValid(laneManager))
	{
		if (laneManager->teamIndex != mc->GetTeamIndex())
			laneManager = laneManager->enemyLane;
	}
}