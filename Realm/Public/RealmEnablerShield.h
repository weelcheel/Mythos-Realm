#pragma once

#include "RealmObjective.h"
#include "RealmEnablerShield.generated.h"

UCLASS()
class ARealmEnablerShield : public ARealmObjective
{
	GENERATED_UCLASS_BODY()

protected:

	/* box collision for the shield */
	UPROPERTY(EditAnywhere, Category = Shield)
	USphereComponent* shieldCollision;

	/* called when a unit starts overlapping the shield (enters the shielded area) */
	UFUNCTION()
	void OnShieldBeginOverlap(AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyInd, bool bFromSweep, const FHitResult& sweepResult);
	
	/* called when a unit stops overlapping the shield (leaves the shielded area) */
	UFUNCTION()
	void OnShieldEndOverlap(AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyInd);
};