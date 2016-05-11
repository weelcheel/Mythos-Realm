#pragma once

#include "RealmObjective.h"
#include "RealmRaider.generated.h"

UCLASS()
class ARaiderCharacter : public ARealmObjective
{
	GENERATED_UCLASS_BODY()

protected:

	/* whether or not this is the first time this raider has died */
	UPROPERTY(BlueprintReadOnly, Category=Raider)
	bool bFirstDeath = false;

	/* timer for when this raider despawns if no one claims it */
	FTimerHandle despawnTimer;

	/* notify the game mode this character died */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage) override;

	/* after the revive delay is finished */
	void OnRaiderRevive();

public:	

	void StartDespawnTimer();
	void OnDespawn();
};