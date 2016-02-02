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

	FText statsDesc;

public:

	/* array of delta stats to add to the player */
	UPROPERTY(EditDefaultsOnly, Category = Mod)
	float deltaStats[(uint8)EStat::ES_Max];

	/* gets the cost of the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 GetCost(bool bNeededCost = false, APlayerCharacter* buyer = nullptr) const;

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

	/* recursively remove this mod and any mods that this recipe requires */
	void RemoveMod(AGameCharacter* character, int32 index);

	/* gets a text description of the stats this mod affects */
	UFUNCTION(BlueprintCallable, Category = Stats)
	FText GetStatsDescription();

	/* gets the default object for a mod class */
	UFUNCTION(BlueprintCallable, Category = Stats)
	static AMod* GetDefaultModObject(TSubclassOf<AMod> modClass);

	/* server blueprint hook for mod use implementation */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ServerModUsed(FHitResult const& hit);

	/* client blueprint hook for mod use implementation */
	UFUNCTION(BlueprintImplementableEvent, Category = UseMod)
	void ClientModUsed(FHitResult const& hit);

	/* determine whether or not this item can be purchased by a certain character based on their credits and if they have the right items */
	UFUNCTION(BlueprintCallable, Category = Stats)
	bool CanCharacterBuyThisMod(APlayerCharacter* buyer);

	/* called when the player actually buys the mod */
	UFUNCTION(BlueprintCallable, Category = Stats)
	void CharacterPurchasedMod(APlayerCharacter* buyer);

	/* called when the character that owns this mod is hurt */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void CharacterDamaged(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AGameCharacter* dmgCauser, AActor* actorCauser, FRealmDamage realmDamage);
};