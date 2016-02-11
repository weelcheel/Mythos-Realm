#pragma once

#include "StatsManager.h"
#include "DamageTypes.h"
#include "Mod.generated.h"

class APlayerCharacter;
class AGameCharacter;
struct FRealmDamage;

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

	/* array of classes of mods this mod requires the purchaser to have */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	TArray<TSubclassOf<AMod> > recipe;

	/* reference to the character that owns this mod, if any */
	UPROPERTY(BlueprintReadOnly, Category = Mod)
	AGameCharacter* characterOwner;

	UPROPERTY()
	FText statsDesc;

	/* timer for cooldowns */
	FTimerHandle cooldownTimer;

public:

	/* array of delta stats to add to the player */
	UPROPERTY(EditDefaultsOnly, Category = Mod)
	float deltaStats[(uint8)EStat::ES_Max];

	/* gets the cost of the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 GetCost(bool bNeededCost = false, APlayerCharacter* buyer = nullptr);

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

	/* recursively finds every mod class that we need for this mod's recipe */
	void GetRecipe(TArray<TSubclassOf<AMod> >& recipeClasses);

	/* gets a text description of the stats this mod affects */
	UFUNCTION(BlueprintCallable, Category = Stats)
	FText GetStatsDescription();

	/* gets the default object for a mod class */
	UFUNCTION(BlueprintCallable, Category = Stats)
	static AMod* GetDefaultModObject(TSubclassOf<AMod> modClass);

	/* server blueprint hook for mod use implementation that gives mouse information at the time of use */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ServerModUsed(const FHitResult& hit);

	/* client blueprint hook for mod use implementation that gives mouse information at the time of use  */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ClientModUsed(const FHitResult& hit);

	/* determine whether or not this item can be purchased by a certain character based on their credits and if they have the right items */
	UFUNCTION(BlueprintCallable, Category = Stats)
	bool CanCharacterBuyThisMod(APlayerCharacter* buyer);

	/* called when the player actually buys the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	void CharacterPurchasedMod(APlayerCharacter* buyer);

	/* called when the character that owns this mod is hurt */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void CharacterDamaged(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AGameCharacter* dmgCauser, AActor* actorCauser, FRealmDamage realmDamage);

	/* called to set the character owner */
	void SetCharacterOwner(AGameCharacter* newOwner)
	{
		characterOwner = newOwner;
	}

	/* start cooldown for the skill */
	UFUNCTION(reliable, NetMulticast, BlueprintCallable, Category=Mod)
	void StartCooldown(float cooldownTime);

	/* gets the percentage of cooldown progress */
	UFUNCTION(BlueprintCallable, Category = Mod)
	float GetCooldownProgressPercent();

	/* gets the amount of time left in the cooldown */
	UFUNCTION(BlueprintCallable, Category = Mod)
	float GetCooldownRemaining();

	/* gets the ui friendly arrray of mod classes needed for the specified mod's recipe */
	UFUNCTION(BlueprintCallable, Category = Mod)
	static void GetUIRecipeForMod(TSubclassOf<AMod> modClass, UPARAM(ref) TArray<TSubclassOf<AMod> >& recipeArray);
};