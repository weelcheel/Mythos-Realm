#pragma once

#include "DamageTypes.generated.h"

UENUM()
enum class ERealmDamageSource: uint8
{
	ERDS_AutoAttack UMETA(DisplayName = "From Auto Attack"),
	ERDS_Skill UMETA(DisplayName = "From Skill"),
	ERDS_Effect UMETA(DisplayName = "From Effect"),
	ERDS_Max UMETA(Hidden),
};

USTRUCT()
struct FRealmDamage
{
	GENERATED_USTRUCT_BODY()

	/* where this damage came from */
	ERealmDamageSource damageSource;

	/* whether or not this was a critical hit */
	bool bCriticalHit;

	FRealmDamage()
	{
		damageSource = ERealmDamageSource::ERDS_AutoAttack;
		bCriticalHit = false;
	}
};

UCLASS(const)
class UPhysicalDamage : public UDamageType
{
	GENERATED_UCLASS_BODY()
};

UCLASS(const)
class USpecialDamage : public UDamageType
{
	GENERATED_UCLASS_BODY()
};

UCLASS(const)
class UTrueDamage : public UDamageType
{
	GENERATED_UCLASS_BODY()
};