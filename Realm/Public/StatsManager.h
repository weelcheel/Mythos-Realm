#pragma once

#include "StatsManager.generated.h"

class AMod;

UENUM(BlueprintType)
enum class EStat : uint8
{
	ES_Atk 	UMETA(DisplayName = "Attack"),
	ES_Def 	UMETA(DisplayName = "Defense"),
	ES_AtkPL UMETA(DisplayName = "Attack per Level"),
	ES_DefPL UMETA(DisplayName = "Defense per Level"),
	ES_SpAtk UMETA(DisplayName = "Special Attack"),
	ES_SpDef UMETA(DisplayName = "Special Defense"),
	ES_SpAtkPL UMETA(DisplayName = "Special Attack per Level"),
	ES_SpDefPL UMETA(DisplayName = "Special Defense per Level"),
	ES_AARange UMETA(DisplayName = "Auto Attack Range"),
	ES_AtkSp UMETA(DisplayName = "Attack Speed"),
	ES_AtkSpPL UMETA(DisplayName = "Attack Speed per Level"),
	ES_Move UMETA(DisplayName = "Movement Speed"),
	ES_CDR UMETA(DisplayName = "Cooldown Reduction"),
	ES_HP UMETA(DisplayName = "Health"),
	ES_HPPL UMETA(DisplayName = "Health per Level"),
	ES_HPRegen UMETA(DisplayName = "Health Regen per Second"),
	ES_HPRegenPL UMETA(DisplayName = "Health Regen PS per Level"),
	ES_Flare UMETA(DisplayName = "Flare"),
	ES_FlareRegen UMETA(DisplayName = "Flare Regen per Second"),
	ES_FlarePL UMETA(DisplayName = "Flare per Level"),
	ES_FlareRegenPL UMETA(DisplayName = "Flare PL per Second"),
	ES_Exp UMETA(DisplayName = "Experience"),
	ES_CritChance UMETA(DisplayName = "Critical Hit Chance"),
	ES_CritRatio UMETA(DisplayName = "Critical Hit Percent Damage"),
	ES_Acc UMETA(DisplayName = "Accuracy"),
	ES_Eva UMETA(DisplayName = "Evasiveness"),
	ES_HPDrain UMETA(DisplayName = "Health Drain"),
	ES_Max UMETA(Hidden)
};

USTRUCT()
struct FEffect
{
	GENERATED_USTRUCT_BODY()

	/* effect name */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FString uiName;

	/* effect description */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FString effectKey;

	/* effect description */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FString description;

	/* stat(s) this effect affects */
	TArray<TEnumAsByte<EStat> > stats;
	
	/* amounts of stats this effects */
	TArray<float> amounts;

	/* duration of the effect */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	float duration;

	/* timer handle for the effect */
	UPROPERTY(BlueprintReadOnly, Category = Effect)
	FTimerHandle effectTimer;

	FEffect()
	{

	}
};

UCLASS()
class AStatsManager : public AActor
{
	friend class AAutoAttackManager;

	GENERATED_UCLASS_BODY()

protected:

	/* whether or not the manager has been initialized */
	UPROPERTY(replicated)
	bool bInitialized;

	/* array of base stats for the character */
	UPROPERTY(replicated)
	float baseStats[(uint8)EStat::ES_Max];

	/* array of mod stats for the character */
	UPROPERTY(replicated)
	float modStats[(uint8)EStat::ES_Max];

	/* array of base stats for the character */
	UPROPERTY(replicated)
	float bonusStats[(uint8)EStat::ES_Max];

	/* current health for the character */
	UPROPERTY(replicated)
	float health;

	/* current health for the character */
	UPROPERTY(replicated)
	float flare;

	/* dynamic array of effects that are currently affecting this character */
	UPROPERTY(replicated)
	TArray<FEffect> effects;

public:

	/* set the health and flare */
	void SetMaxHealth();
	void SetMaxFlare();

	/* initialize the stats manager with a character's base stats */
	void InitializeStats(float* initBaseStats);

	/* gets the current value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetCurrentValueForStat(EStat stat) const;

	/* gets the base value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetBaseValueForStat(EStat stat) const;

	/* get the unaffected value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetUnaffectedValueForStat(EStat stat) const;

	/* update the bonus stats with stats from mods. called each time the mods array updates */
	void UpdateModStats(TArray<AMod*>& mods);

	/* add buff/debuff to the player's stats */
	void AddEffect(FString effectName, FString effectDescription, FString effectKey, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration = 0.f);

	/* when an effect with a duration finishes */
	UFUNCTION()
	void EffectFinished(FString key)
	{
		int32 effectInd = -1;

		for (int32 i = 0; i < effects.Num(); i++)
		{
			if (effects[i].effectKey == key)
				effectInd = i;
		}

		if (effectInd < 0)
			return;

		if (Role == ROLE_Authority)
		{
			int32 ind = 0;
			for (TEnumAsByte<EStat> eStat : effects[effectInd].stats)
			{
				bonusStats[(uint8)eStat.GetValue()] -= effects[effectInd].amounts[ind];
				ind++;
			}
		}

		effects.RemoveAt(effectInd);
	}

	/* get the effects array */
	UFUNCTION(BlueprintCallable, Category = Effects)
	void GetEffects(TArray<FEffect>& outEffects)
	{
		outEffects = effects;
	}

	/* take damage and update health accordingly */
	void RemoveHealth(float amount);

	/* use or add flare*/
	void RemoveFlare(float amount);

	/* get health */
	float GetHealth() const;

	/* get flare */
	float GetFlare() const;
};