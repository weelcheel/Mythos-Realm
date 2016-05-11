#pragma once

#include "GameCharacter.h"
#include "PlayerCharacter.generated.h"

const static int32 CREDIT_CHANGE_MAX = 5000;

UCLASS(ABSTRACT, Blueprintable)
class APlayerCharacter : public AGameCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* amount of credits the player has to spend at the Mod Shop */
	UPROPERTY(EditInstanceOnly, Category = Credits, replicated)
	float credits;

	/* array of hits for this life for death recaps */
	UPROPERTY(BlueprintReadOnly, replicated, Category=Hits)
	TArray<FTakeHitInfo> lifeHits;

	/* timer to clear out hit info. this is reset every time this player takes damage */
	FTimerHandle liftHitsClearTimer;

	/* timer for ambient credit income */
	FTimerHandle ambientCreditTimer;

	/* amount for ambient credit income per second */
	float ambientCreditAmount;

	/* back timer */
	UPROPERTY(BlueprintReadOnly, Category = BaseTeleport)
	FTimerHandle baseTeleportTimer;

	/* function to clear out all of the hits for this player */
	void ClearLifeHits();

	/* record hits for death recaps */
	virtual void ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled, FRealmDamage& realmDamage) override;

	/* let the specific classes have different character overlays */
	//virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

	/* override for respawns */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage) override;

	/* on ambient credit income */
	void OnAmbientCreditIncome();

	/* respawn players */
	void Respawn();

public:

	/* add/spend credits */
	UFUNCTION(BlueprintCallable, Category=Player)
	void ChangeCredits(int32 deltaAmount, const FVector& worldLoc = FVector::ZeroVector);

	/* get credits */
	UFUNCTION(BlueprintCallable, Category = Player)
	int32 GetCredits() const
	{
		return credits;
	}

	/* starts the ambient credit income */
	void StartAmbientCreditIncome(int32 amount);

	/* starts the respawn timers */
	UFUNCTION(reliable, NetMulticast)
	void StartRespawnTimers(float respawnTime);

	/* starts the backing sequence to teleport this player back to base */
	UFUNCTION(reliable, NetMulticast)
	void StartBaseTeleport();

	/* stops the backing sequence */
	UFUNCTION(reliable, NetMulticast)
	void StopBaseTeleport();

	/* actually perform the base teleport */
	UFUNCTION(reliable, NetMulticast)
	void PerformBaseTeleport();
};