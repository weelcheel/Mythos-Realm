#pragma once

#include "StatsManager.h"
#include "Effect.generated.h"

UCLASS(Blueprintable)
class AEffect : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	UFUNCTION()
	void OnRepDuration();

public:

	/* effect name */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	FString uiName;

	/* effect description */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Effect)
	FString description;

	/* stat(s) this effect affects */
	TArray<TEnumAsByte<EStat> > stats;

	/* amounts of stats this effects */
	TArray<float> amounts;

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
};