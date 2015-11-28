#pragma once

#include "StatsManager.h"
#include "Mod.generated.h"

UCLASS()
class AMod : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/* cost of the mod */
	UPROPERTY(EditDefaultsOnly, Category = Mod)
	int32 cost;

	/* name of the mod */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	FText modName;

public:

	/* array of delta stats to add to the player */
	UPROPERTY(EditDefaultsOnly, Category = Mod)
	float deltaStats[(uint8)EStat::ES_Max];

	/* gets the cost of the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 GetCost() const
	{
		return cost;
	}
};