#pragma once

#include "RealmObjective.h"
#include "RealmEnablerShieldGenerator.generated.h"

class ARealmEnablerShield;

UCLASS()
class ARealmEnablerShieldGenerator : public ARealmObjective
{
	GENERATED_UCLASS_BODY()

protected:

	/* shield that this is a generator for */
	UPROPERTY(EditAnywhere, Category = Shield)
	ARealmEnablerShield* shield;

	/* override for respawns */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser) override;

	/* respawn the shield generator */
	void Respawn();
};