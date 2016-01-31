#include "Realm.h"
#include "RealmEnablerShieldGenerator.h"

ARealmEnablerShieldGenerator::ARealmEnablerShieldGenerator(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	GetCharacterMovement()->SetMovementMode(MOVE_None);
}

void ARealmEnablerShieldGenerator::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage);

	if (Role == ROLE_Authority)
	{
		if (IsValid(shield) && shield->IsAlive())
			shield->GetStatsManager()->bonusStats[(uint8)EStat::ES_HP] -= 5000.f;
	}

	GetWorldTimerManager().SetTimer(respawnTimer, this, &ARealmEnablerShieldGenerator::Respawn, 45.f);
}

void ARealmEnablerShieldGenerator::Respawn()
{
	if (Role == ROLE_Authority)
	{
		if (IsValid(shield) && shield->IsAlive())
			shield->GetStatsManager()->bonusStats[(uint8)EStat::ES_HP] += 5000.f;
	}

	bIsDying = false;
}