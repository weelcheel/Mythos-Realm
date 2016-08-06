#pragma once

#include "GameCharacterData.generated.h"

UCLASS(ABSTRACT, Blueprintable)
class UGameCharacterData : public UObject
{
	friend class AGameCharacter;

	GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float attack;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float defense;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float attackPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float defensePerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float specialAttack;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float specialDefense;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float specialAttackPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float specialDefensePerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float attackSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float attackSpeedPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float movementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float health;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float healthPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float healthRegen;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float healthRegenPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float flare;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float flarePerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float flareRegen;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float flareRegenPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Portrait)
	UTexture2D* portrait;

public:

	/* gets an array of stats to use for the game character class */
	float* GetCharacterBaseStats() const;
};