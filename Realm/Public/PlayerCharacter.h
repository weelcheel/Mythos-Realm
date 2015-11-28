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
	UPROPERTY()
	TArray<FTakeHitInfo> lifeHits;

	/* timer for ambient credit income */
	FTimerHandle ambientCreditTimer;

	/* amount for ambient credit income per second */
	int32 ambientCreditAmount;

	/* record hits for death recaps */
	virtual void ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled) override;

	/* let the specific classes have different character overlays */
	virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

	/* override for respawns */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser) override;

	/* on ambient credit income */
	void OnAmbientCreditIncome();

	/* respawn players */
	void Respawn();

public:

	/* add/spend credits */
	UFUNCTION(BlueprintCallable, Category=Player)
	void ChangeCredits(int32 deltaAmount);

	/* get credits */
	UFUNCTION(BlueprintCallable, Category = Player)
	int32 GetCredits() const
	{
		return credits;
	}

	/* starts the ambient credit income */
	void StartAmbientCreditIncome(int32 amount);
};