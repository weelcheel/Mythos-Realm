#pragma once

#include "StatsManager.h"
#include "Effect.generated.h"

UCLASS(Blueprintable)
class AEffect : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/* whether or not this timer needs to replicate */
	UPROPERTY(ReplicatedUsing = OnRepTimerReset)
	bool bTimerReset;

	UFUNCTION()
	void OnRepDuration();

	UFUNCTION()
	void OnRepTimerReset();

public:

	/* effect name */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	FText uiName;

	/* effect description */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	FText description;

	/* stat(s) this effect affects */
	UPROPERTY()
	TArray<TEnumAsByte<EStat> > stats;

	/* amounts of stats this effects */
	UPROPERTY()
	TArray<float> amounts;

	/* stats owner of this effect */
	UPROPERTY()
	AStatsManager* statsManager;

	/* is this effect transfered to a player killer? */
	UPROPERTY(BlueprintReadWrite, Category=Effect)
	bool bIsTransferredToPlayerKiller = false;

	/* duration of the effect */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect, ReplicatedUsing = OnRepDuration)
	float duration;

	/* whether or not this is a stacking effect */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	bool bStacking;

	/* amount of stacks this effect has (if can stack) */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	int32 stackAmount;

	/* timer handle for the effect */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FTimerHandle effectTimer;

	/* name of the effect for game reasons */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FString keyName;

	/* whether or not this effect can be added multiple times to one character */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	bool bCanBeInflictedMultipleTimes;

	/* reset this effect's timer and change it if needed */
	UFUNCTION(BlueprintCallable, Category = Timer)
	void ResetEffectTimer(float newTime = 0.f);
};