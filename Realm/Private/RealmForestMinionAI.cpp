#include "Realm.h"
#include "RealmForestMinionAI.h"

ARealmForestMinionAI::ARealmForestMinionAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void ARealmForestMinionAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	AMinionCharacter* mc = Cast<AMinionCharacter>(InPawn);
	if (mc)
		mc->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	mc->GetStatsManager()->SetMaxHealth();

	minionCharacter = mc;

	FTimerHandle f;
	GetWorldTimerManager().SetTimer(f, this, &ARealmForestMinionAI::ReevaluateTargets, 0.8f, true);
}

void ARealmForestMinionAI::CharacterTookDamage(AGameCharacter* damageCauser)
{
	if (!IsValid(damageCauser) || !IsValid(minionCharacter) || bReturningHome) //ignore damagers if were on our way home
		return;

	if (!IsValid(minionCharacter->GetCurrentTarget()))
	{
		minionCharacter->SetCurrentTarget(damageCauser);
		minionCharacter->StartAutoAttack();
	}
}

void ARealmForestMinionAI::ReevaluateTargets()
{
	if (!IsValid(minionCharacter))
		return;

	if (bReturningHome)
	{
		if (minionCharacter->GetActorLocation().Equals(homePosition, 20.f))
			bReturningHome = false;

		return;
	}

	if (IsValid(minionCharacter->GetCurrentTarget()))
	{
		float distsq = (minionCharacter->GetCurrentTarget()->GetActorLocation() - GetCharacter()->GetActorLocation()).SizeSquared2D();
		if (!minionCharacter->GetCurrentTarget()->IsAlive() || distsq > FMath::Square(550.f))
			NeedsNewCommand();
		else
			minionCharacter->StartAutoAttack();
	}
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