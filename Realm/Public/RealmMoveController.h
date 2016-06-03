#pragma once

#include "AIController.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "RealmMoveController.generated.h"

class AGameCharacter;

UCLASS()
class ARealmMoveController : public AAIController
{
	GENERATED_UCLASS_BODY()

protected:

	/* the radius or range that an enemy has to enter to be able to be targeted by this minion */
	UPROPERTY(VisibleAnywhere, Category = Minion)
	UPawnSensingComponent* targetRadius;

	/* array of enemy units that are currently in range of this minion */
	TArray<AGameCharacter*> inRangeTargets;

	/* called whenever an enemy walks into attack radius */
	UFUNCTION()
	virtual void OnTargetEnterRadius(class APawn* seenPawn);

public:

	/* whenever the character stops auto attacking and needs a new command */
	virtual void NeedsNewCommand();

	virtual void CharacterDamaged(AGameCharacter* damageCauser);

	virtual void CharacterInAttackRange();
	virtual void Possess(APawn* inPawn) override;

	/* called whenever the game has ended */
	void GameEnded();
};