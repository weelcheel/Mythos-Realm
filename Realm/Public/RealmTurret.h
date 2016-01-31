#pragma once

#include "RealmObjective.h"
#include "DamageInstance.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmTurret.generated.h"

UCLASS()
class ATurret : public ARealmObjective
{
	GENERATED_UCLASS_BODY()

protected:

	/* array of enemy units that are currently in range of this turret */
	TArray<AGameCharacter*> inRangeTargets;

	/* override the check auto attack function to make sure the turret doesn't move */
	virtual void CheckAutoAttack() override;

	/* current target out of range, so find new target */
	void TargetOutofRange();

	/* let the specific classes have different character overlays */
	virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

	/* notify the game mode this turret has been destroyed */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage) override;
public:

	/* called when a component overlaps, sets the first enemy that enters this radius as the current target */
	UFUNCTION()
	void OnSeePawn(APawn* OtherActor);

	virtual void ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget) override;
};