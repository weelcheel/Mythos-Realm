#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmForestMinionAI.generated.h"

class ALaneManager;
class AGameCharacter;
class AMinionCharacter;

UCLASS()
class ARealmForestMinionAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the character casted to a minion character */
	UPROPERTY()
	AMinionCharacter* minionCharacter;

	/* lane manager that controls this minion */
	UPROPERTY(VisibleAnywhere, Category = Lane)
	ALaneManager* laneManager;

	/* current enemy unit this target is attacking (if any) */
	AGameCharacter* enemyTarget;

	/* called whenever an enemy walks into attack radius */
	UFUNCTION()
	void OnTargetEnterRadius(class APawn* seenPawn);

public:

	virtual void Possess(APawn* InPawn) override;
};