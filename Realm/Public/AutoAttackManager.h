#pragma once

#include "AutoAttackManager.generated.h"

class UStatsManager;
class AProjectile;

USTRUCT()
struct FAutoAttack
{
	GENERATED_USTRUCT_BODY()

	/* range of the auto attack */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	float attackRange;

	/* animation to play on the character when we attack */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	UAnimMontage* attackAnimation;

	/* class of the projectile to spawn for this auto attack */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	TSubclassOf<AProjectile> projectileClass;

	/* name of the socket located where we want to spawn the projectile */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	FName projectileSocket;

	/* whether or not this auto attack launches a projectile */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	bool bProjectile;

	/* length of time it takes for the attack to launch */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	float launchTime;

	/* sound to play when the attack is launched */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	USoundCue* attackLaunchSound;

	/* sound to play when the attack makes contact */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	USoundCue* attackHitSound;
};

UCLASS()
class UAutoAttackManager : public UObject
{
	GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(replicated)
	TArray<FAutoAttack> autoAttacks;

	UPROPERTY(replicated)
	int32 currentAttackIndex;

public:

	/* initialize the attack manager */
	void InitializeManager(TArray<FAutoAttack>& attacks, UStatsManager* stats);

	/* get the range of the current auto attack */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	float GetCurrentAutoAttackRange() const;

	/* get the range of the current auto attack squared */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	float GetCurrentAutoAttackRangeSquared() const;

	/* get gets whether or not the current attack launches a projectile */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	bool IsCurrentAttackProjectile() const;

	/* get gets whether or not the current attack launches a projectile */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	UAnimMontage* GetCurrentAttackAnimation() const;

	/* get the attack projectile class */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	TSubclassOf<AProjectile> GetCurrentAutoAttackProjectileClass() const;

	/* get the attack projectile socket */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	FName GetCurrentAutoAttackProjectileSocket() const;

	/* get the attack's current launch time */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	float GetAutoAttackLaunchTime() const;

	/* lets the auto attack index of the character to be set */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	void SetAutoAttackIndex(int32 newIndex);

	/* gets the auto attack launch sound */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	USoundCue* GetCurrentAutoAttackLaunchSound() const;

	/* gets the auto attack hit sound */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	USoundCue* GetCurrentAutoAttackHitSound() const;

	/* override this so our manager can replicate with the character owner */
	virtual bool IsSupportedForNetworking() const override;
};