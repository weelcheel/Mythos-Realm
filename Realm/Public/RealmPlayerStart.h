#pragma once

#include "GameFramework/PlayerStart.h"
#include "RealmPlayerStart.generated.h"

UCLASS()
class ARealmPlayerStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

public:

	/* what team can spawn here */
	UPROPERTY(EditInstanceOnly, Category = Team)
	int32 teamIndex;

	/* what memeber of the team spawns here */
	UPROPERTY(EditInstanceOnly, Category = Team)
	int32 indSpawnIndex;
};