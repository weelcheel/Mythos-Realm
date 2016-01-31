#pragma once

#include "GameCharacter.h"
#include "MinionCharacter.generated.h"

UCLASS()
class AMinionCharacter : public AGameCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* amount of money the destroying player receives for destroying this objective. */
	UPROPERTY(EditAnywhere, Category = Reward)
	int32 playerReward;

	/* override for destruction and rewards */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage) override;

	/* let the specific classes have different character overlays */
	virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

public:

	/* called whenever a minion is under attack and needs help, if we aren't already helping try to help */
	virtual void ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget) override;

	/* let ai know we got hurt */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
};