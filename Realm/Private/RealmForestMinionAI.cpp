#include "Realm.h"
#include "RealmForestMinionAI.h"

ARealmForestMinionAI::ARealmForestMinionAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	aggroDistance = 1000.f;
}

void ARealmForestMinionAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	AMinionCharacter* mc = Cast<AMinionCharacter>(InPawn);
	if (mc)
		mc->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	mc->GetStatsManager()->SetMaxHealth();

	minionCharacter = mc;
}

void ARealmForestMinionAI::CharacterTookDamage(AGameCharacter* damageCauser)
{
	if (!IsValid(damageCauser) || !IsValid(minionCharacter) || bReturningHome) //ignore damagers if were on our way home
		return;

	if (!IsValid(minionCharacter->GetCurrentTarget()))
	{
		minionCharacter->SetCurrentTarget(damageCauser);
		minionCharacter->StartAutoAttack();

		GetWorldTimerManager().SetTimer(reevalTimer, this, &ARealmForestMinionAI::ReevaluateTargets, 0.8f, true);
	}
}

void ARealmForestMinionAI::ReevaluateTargets()
{
	if (!IsValid(minionCharacter))
		return;

	if (bReturningHome)
	{
		if (minionCharacter->GetActorLocation().Equals(homePosition, 20.f))
		{
			bReturningHome = false;
			GetWorldTimerManager().ClearTimer(reevalTimer);
		}
		else
			MoveToLocation(homePosition);

		return;
	}

	if (IsValid(minionCharacter->GetCurrentTarget()))
	{
		float distsq = (minionCharacter->GetActorLocation() - minionCharacter->GetCurrentTarget()->GetActorLocation()).SizeSquared2D();
		if (!minionCharacter->GetCurrentTarget()->IsAlive() || !minionCharacter->CanSeeOtherCharacter(minionCharacter->GetCurrentTarget()) || !minionCharacter->GetCurrentTarget()->IsTargetable() || distsq > FMath::Square(aggroDistance))
			NeedsNewCommand();
		else if (distsq > FMath::Square(minionCharacter->GetCurrentValueForStat(EStat::ES_AARange)))
			MoveToActor(minionCharacter->GetCurrentTarget(), minionCharacter->GetCurrentValueForStat(EStat::ES_AARange));
		else
			minionCharacter->StartAutoAttack();
	}
	else
		NeedsNewCommand();
}

void ARealmForestMinionAI::NeedsNewCommand()
{
	if (!IsValid(minionCharacter))
		return;

	//out of range, so just head back home and ignore the haters
	//@todo: give some protection for heading back to camp (maybe)
	minionCharacter->SetCurrentTarget(nullptr);
	minionCharacter->StopAutoAttack();
	MoveToLocation(homePosition);
	bReturningHome = true;
}