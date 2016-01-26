#pragma once

#include "GameCharacter.h"
#include "RealmObjective.generated.h"

UCLASS(ABSTRACT)
class ARealmObjective : public AGameCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* amount of money the destroying team receives (including the destroying player) for destroying this objective. */
	UPROPERTY(EditAnywhere, Category = Reward)
	int32 teamReward;

public:
	/* amount of money the destroying player receives for destroying this objective. */
	UPROPERTY(EditAnywhere, Category = Reward)
	int32 playerReward;

	/* called by the server every time a player character hurts another player character. give them aggro if they're in range */
	virtual void CheckDamage(FTakeHitInfo& damage);
};