#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmRaiderAI.generated.h"

class ARaiderCharacter;
class AGameCharacter;
class ARealmObjective;

UCLASS()
class ARealmRaiderAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the character casted to a raider character */
	UPROPERTY()
	ARaiderCharacter* raiderCharacter;

	/* current objective that this unit is trying to move to */
	ARealmObjective* objectiveTarget;

	/* current enemy unit this target is attacking (if any) */
	AGameCharacter* enemyTarget;

	/* called whenever an enemy walks into attack radius */
	UFUNCTION()
	void OnTargetEnterRadius(class APawn* seenPawn);

public:

	virtual void Possess(APawn* InPawn) override;
};