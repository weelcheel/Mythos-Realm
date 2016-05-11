#include "Realm.h"
#include "RealmRaider.h"
#include "RealmGameMode.h"
#include "RealmRaiderAI.h"

ARaiderCharacter::ARaiderCharacter(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	teamIndex = 3;
}

void ARaiderCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	if (Role == ROLE_Authority)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(InstigatingPawn);
		if (!gc)
			return;

		GetWorld()->GetAuthGameMode<ARealmGameMode>()->OnRaiderDeath();
		if (bFirstDeath)
		{
			GetWorldTimerManager().ClearAllTimersForObject(GetController());
			SetLifeSpan(5.f);
			GetController()->SetLifeSpan(5.f);
		}
		else
		{
			bFirstDeath = true;
			SetTeamIndex(gc->GetTeamIndex());

			ARealmRaiderAI* controller = Cast<ARealmRaiderAI>(GetController());
			if (IsValid(controller))
				controller->OnDeath();

			bIsDying = true;

			FTimerHandle reviveDelay;
			GetWorldTimerManager().SetTimer(reviveDelay, this, &ARaiderCharacter::OnRaiderRevive, 5.f, false);
			GetWorldTimerManager().ClearTimer(despawnTimer);
			return;
		}
	}

	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage);
}

void ARaiderCharacter::OnRaiderRevive()
{
	GetStatsManager()->baseStats[(uint16)EStat::ES_HP] -= GetStatsManager()->baseStats[(uint16)EStat::ES_HP] * 0.25f;
	GetStatsManager()->SetMaxHealth();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bIsDying = false;

	StartDespawnTimer();
}

void ARaiderCharacter::StartDespawnTimer()
{
	GetWorldTimerManager().SetTimer(despawnTimer, this, &ARaiderCharacter::OnDespawn, 120.f, false);
}

void ARaiderCharacter::OnDespawn()
{
	StopAutoAttack();

	bFirstDeath = true;
	FDamageEvent dmg;
	FRealmDamage rdmg;
	Die(0, dmg, this, this, rdmg);
}