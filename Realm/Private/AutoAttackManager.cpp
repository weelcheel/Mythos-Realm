#include "Realm.h"
#include "AutoAttackManager.h"
#include "UnrealNetwork.h"
#include "StatsManager.h"

UAutoAttackManager::UAutoAttackManager(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	
}

void UAutoAttackManager::InitializeManager(TArray<FAutoAttack>& attacks, UStatsManager* stats)
{
	autoAttacks = attacks;
	
	if (IsValid(stats) && autoAttacks.Num() > 0)
		stats->baseStats[(uint8)EStat::ES_AARange] = autoAttacks[0].attackRange;
}

float UAutoAttackManager::GetCurrentAutoAttackRange() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackRange;
	else
		return -1.f;
}

float UAutoAttackManager::GetCurrentAutoAttackRangeSquared() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackRange * autoAttacks[currentAttackIndex].attackRange;
	else
		return -1.f;
}

UAnimMontage* UAutoAttackManager::GetCurrentAttackAnimation() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackAnimation;
	else
		return nullptr;
}

bool UAutoAttackManager::IsCurrentAttackProjectile() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].bProjectile;
	else
		return false;
}

TSubclassOf<AProjectile> UAutoAttackManager::GetCurrentAutoAttackProjectileClass() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].projectileClass;
	else
		return nullptr;
}

FName UAutoAttackManager::GetCurrentAutoAttackProjectileSocket() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].projectileSocket;
	else
		return TEXT("");
}

float UAutoAttackManager::GetAutoAttackLaunchTime() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].launchTime;
	else
		return -1.f;
}

void UAutoAttackManager::SetAutoAttackIndex(int32 newIndex)
{
	if (newIndex >= 0 && newIndex < autoAttacks.Num())
		currentAttackIndex = newIndex;
}

USoundCue* UAutoAttackManager::GetCurrentAutoAttackLaunchSound() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackLaunchSound;
	else
		return nullptr;
}

USoundCue* UAutoAttackManager::GetCurrentAutoAttackHitSound() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackHitSound;
	else
		return nullptr;
}

bool UAutoAttackManager::IsSupportedForNetworking() const
{
	return true;
}

void UAutoAttackManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	DOREPLIFETIME(UAutoAttackManager, autoAttacks);
	DOREPLIFETIME(UAutoAttackManager, currentAttackIndex);
}