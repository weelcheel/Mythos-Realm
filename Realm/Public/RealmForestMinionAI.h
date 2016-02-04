#pragma once

#include "RealmMoveController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmForestMinionAI.generated.h"

class ALaneManager;
class AGameCharacter;
class AMinionCharacter;
class AForestCamp;

UCLASS()
class ARealmForestMinionAI : public ARealmMoveController
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the character casted to a minion character */
	UPROPERTY()
	AMinionCharacter* minionCharacter;

	/* whether or not this unit repositioned from being to close to a friendly */
	bool bRepositioned = false;

	/* whether or not this minion is on his way back home */
	bool bReturningHome = false;

	/* target out of aggro range */
	void ReevaluateTargets();

public:

	/* home vector of this minion */
	FVector homePosition;

	/* home camp spawner */
	UPROPERTY(BlueprintReadOnly, Category=CampSpawner)
	AForestCamp* campSpawner;

	virtual void Possess(APawn* InPawn) override;
	virtual void NeedsNewCommand() override;

	/* character takee damage */
	void CharacterTookDamage(AGameCharacter* damageCauser);
};