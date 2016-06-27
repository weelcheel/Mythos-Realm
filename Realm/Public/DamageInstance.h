#pragma once

#include "DamageTypes.h"
#include "DamageInstance.generated.h"

/** replicated information on a hit we've taken */
USTRUCT(BlueprintType)
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

	/** The amount of damage actually applied */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	float ActualDamage;

	/** The damage type we were hit with. */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	UClass* DamageTypeClass;

	/** Who hit us */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	class AGameCharacter* PawnInstigator;

	/** Who actually caused the damage */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	TWeakObjectPtr<class AActor> DamageCauser;

	/** Specifies which DamageEvent below describes the damage received. */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	int32 DamageEventClassID;

	/** Rather this was a kill */
	UPROPERTY(BlueprintReadOnly, Category = Hits)
	uint32 bKilled : 1;

private:

	/** A rolling counter used to ensure the struct is dirty and will replicate. */
	UPROPERTY()
		uint8 EnsureReplicationByte;

	/** Describes general damage. */
	UPROPERTY()
		FDamageEvent GeneralDamageEvent;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FRealmDamage realmDamage;

	/* ui name for this damage event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FText damageName;

	/* class of the game character that created this damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	UClass* characterClass;

	FTakeHitInfo()
		: ActualDamage(0)
		, DamageTypeClass(NULL)
		, PawnInstigator(NULL)
		, DamageCauser(NULL)
		, DamageEventClassID(0)
		, bKilled(false)
		, EnsureReplicationByte(0)
	{}

	FDamageEvent& GetDamageEvent()
	{
		if (GeneralDamageEvent.DamageTypeClass == NULL)
		{
			GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
		}
		return GeneralDamageEvent;
	}

	void SetDamageEvent(const FDamageEvent& DamageEvent)
	{
		DamageEventClassID = DamageEvent.GetTypeID();
		GeneralDamageEvent = DamageEvent;

		DamageTypeClass = DamageEvent.DamageTypeClass;
	}

	void EnsureReplication()
	{
		EnsureReplicationByte++;
	}
};