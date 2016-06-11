#pragma once

#include "ShieldManager.generated.h"

class AGameCharacter;

USTRUCT()
struct FCharacterShield
{
	GENERATED_USTRUCT_BODY()

	/* amount of damage this shield absorbs */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Shield)
	float amountMax;

	/* amount of shield left in this shield */
	float amount;

	/* duration of this shield. <= 0.f for indefinitely lasting shields */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shield)
	float duration;

	/* timer handle for this shield */
	UPROPERTY(BlueprintReadOnly, Category = Shield)
	FTimerHandle timer;

	/* types of damage this shield absorbs */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shield)
	TArray<TSubclassOf<UDamageType> > damageTypes;

	/* keyname for this shield */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shield)
	FString key;

	/* game character that created (originated) this shield */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shield)
	AGameCharacter* originatingCharacter;
};

UCLASS()
class AShieldManager : public AActor
{
	GENERATED_UCLASS_BODY()

	/* array of shields this character currently has */
	UPROPERTY()
	TMap<FString, FCharacterShield> shields;

	/* replicated total shield amount for things like UI */
	UPROPERTY(replicated)
	float totalShieldAmount = 0.f;

protected:

	/* updates the total shield amount */
	void UpdateTotalShieldAmount();

public:
	
	/* reference to the character that owns this manager */
	UPROPERTY()
	AGameCharacter* owningCharacter;

	/* adds a shield to the array */
	UFUNCTION(BlueprintCallable, Category = Shield)
	void AddShield(FCharacterShield newShield);

	/* whether or not this character has any shields to absorb damaage */
	bool CanAbsorbDamage() const;

	/* try to absorb damage and then return any damage not absorbed */
	float TryAbsorbDamage(float dmgAmount, TSubclassOf<UDamageType> dmgType);

	/* returns the total amount of damage that can be absorbed by this character's shields */
	UFUNCTION(BlueprintCallable, Category = Effect)
	float GetTotalShieldAmount() const;

	/* called when a shield should finish */
	UFUNCTION(BlueprintCallable, Category = Effect)
	void ShieldFinished(FCharacterShield finishingShield);

	/* goes through the shields and determines whether or not the specified character has applied any shield to this character. not a fast function since we have to iterate over the entire hash map */
	UFUNCTION(BlueprintCallable, Category = Shield)
	bool DoesContainCharactersShield(AGameCharacter* originatingUnit);
};