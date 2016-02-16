#include "Realm.h"
#include "AutoAttackManager.h"
#include "UnrealNetwork.h"
#include "StatsManager.h"

AAutoAttackManager::AAutoAttackManager(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AAutoAttackManager::InitializeManager(TArray<FAutoAttack>& attacks, AStatsManager* stats)
{
	autoAttacks = attacks;
	
	if (IsValid(stats) && autoAttacks.Num() > 0)
		stats->baseStats[(uint8)EStat::ES_AARange] = autoAttacks[0].attackRange;
}

float AAutoAttackManager::GetCurrentAutoAttackRange() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackRange;
	else
		return -1.f;
}

float AAutoAttackManager::GetCurrentAutoAttackRangeSquared() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackRange * autoAttacks[currentAttackIndex].attackRange;
	else
		return -1.f;
}

UAnimMontage* AAutoAttackManager::GetCurrentAttackAnimation() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackAnimation;
	else
		return nullptr;
}

bool AAutoAttackManager::IsCurrentAttackProjectile() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].bProjectile;
	else
		return false;
}

TSubclassOf<AProjectile> AAutoAttackManager::GetCurrentAutoAttackProjectileClass() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].projectileClass;
	else
		return nullptr;
}

FName AAutoAttackManager::GetCurrentAutoAttackProjectileSocket() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].projectileSocket;
	else
		return TEXT("");
}

float AAutoAttackManager::GetAutoAttackLaunchTime() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].launchTime;
	else
		return -1.f;
}

void AAutoAttackManager::SetAutoAttackIndex(int32 newIndex)
{
	if (newIndex >= 0 && newIndex < autoAttacks.Num())
		currentAttackIndex = newIndex;
}

USoundCue* AAutoAttackManager::GetCurrentAutoAttackLaunchSound() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackLaunchSound;
	else
		return nullptr;
}

USoundCue* AAutoAttackManager::GetCurrentAutoAttackHitSound() const
{
	if (currentAttackIndex < autoAttacks.Num())
		return autoAttacks[currentAttackIndex].attackHitSound;
	else
		return nullptr;
}

void AAutoAttackManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAutoAttackManager, autoAttacks);
	DOREPLIFETIME(AAutoAttackManager, currentAttackIndex);
}