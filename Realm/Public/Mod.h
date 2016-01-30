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

	/* description of the mod */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	FText modDescription;

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

	/* gets the description of the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	FText GetDescription() const
	{
		return modDescription;
	}

	/* gets the name of the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	FText GetModName() const
	{
		return modName;
	}

	/* gets a text description of the stats this mod affects */
	UFUNCTION(BlueprintCallable, Category = Stats)
	FString GetStatsDescription() const;

	/* gets the default object for a mod class */
	UFUNCTION(BlueprintCallable, Category = Stats)
	static AMod* GetDefaultModObject(TSubclassOf<AMod> modClass);

	/* server blueprint hook for mod use implementation */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ServerModUsed(FHitResult const& hit);

	/* client blueprint hook for mod use implementation */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ClientModUsed(FHitResult const& hit);

	/* validate a target for this mod */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ValidateTarget(FHitResult const& hit);
};