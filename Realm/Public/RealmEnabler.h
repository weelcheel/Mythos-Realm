#pragma once

#include "RealmObjective.h"
#include "RealmEnabler.generated.h"

class ARealmPlayerController;

UCLASS()
class ARealmEnabler : public ARealmObjective
{
	GENERATED_UCLASS_BODY()

protected:

	/* effect we give to all of our in range allies */
	AEffect* enablerAuraEffect;
	
	/* range from location this enabler protects allies */
	UPROPERTY(EditDefaultsOnly, Category = Enabler)
	float auraRange;

	/* current protected units */
	TArray<APlayerCharacter*> protectedPlayers;

	/* override for destruction and rewards */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc) override;

	/* let the specific classes have different character overlays */
	//virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

	/* set the aura timer */
	virtual void BeginPlay() override;
	void OnTargetsUpdate();

	/* function to go through and remove all aura effects */
	void EnablerEffectFinished(AGameCharacter* gc);

public:

	/* called whenever the player clicks on the store. we send back details of what to load and a reference to us */
	void PlayerOpenedStore(ARealmPlayerController* pc);

};