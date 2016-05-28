#pragma once

#include "DamageTypes.h"
#include "ModManager.generated.h"

UCLASS()
class UModManager : public UObject
{
	friend class AGameCharacter;

	GENERATED_UCLASS_BODY()

	/* reference to the character this is a manager for */
	UPROPERTY()
	AGameCharacter* managedCharacter;

public:

	/* whenever this character is damaged, let the mods know */
	void CharacterDamaged(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AGameCharacter* dmgCauser, AActor* actorCauser, FRealmDamage realmDamage);
	void CharacterDealtDamage(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AActor* actorCauser, FRealmDamage realmDamage, AGameCharacter* targetCharacter);

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
};