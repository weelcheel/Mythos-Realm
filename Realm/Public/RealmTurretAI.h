#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmTurretAI.generated.h"

class ALaneManager;
class AGameCharacter;
class AMinionCharacter;
class ARealmObjective;

UCLASS()
class ARealmTurretAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* for sensing pawns that come into our radius */
	UPROPERTY(EditDefaultsOnly, Category = Sensing)
	UPawnSensingComponent* pawnSensor;

	/* called when a component overlaps, sets the first enemy that enters this radius as the current target */
	UFUNCTION()
	void OnSeePawn(APawn* OtherActor);
};