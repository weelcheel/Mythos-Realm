#pragma once

#include "DamageTypes.generated.h"

UENUM()
enum class ERealmDamageSource: uint8
{
	ERDS_AutoAttack UMETA(DisplayName = "From Auto Attack"),
	ERDS_Skill UMETA(DisplayName = "From Skill"),
	ERDS_Effect UMETA(DisplayName = "From Effect"),
	ERDS_Turret UMETA(DisplayName = "From Turret"),
	ERDS_Max UMETA(Hidden),
};

USTRUCT(BlueprintType)
struct FRealmDamage
{
	GENERATED_USTRUCT_BODY()

	/* where this damage came from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	ERealmDamageSource damageSource;

	/* whether or not this was a critical hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCriticalHit;

	/* the damaging character was controlled by a player when they incurred the damage ALAWAYS CHECK THIS FOR VALIDITY */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	AGameCharacter* controllingCharacter;

	FRealmDamage()
	{
		damageSource = ERealmDamageSource::ERDS_AutoAttack;
		bCriticalHit = false;
	}
};

USTRUCT(BlueprintType)
struct FDamageRecap
{
	GENERATED_USTRUCT_BODY()

	/* ui name for this damage event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FText damageName;

	/* class of the game character that created this damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	UClass* characterClass;

	/* damage icon to display in ui */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	UTexture2D* damageImage;
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