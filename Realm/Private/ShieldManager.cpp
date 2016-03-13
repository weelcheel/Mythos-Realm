#include "Realm.h"
#include "ShieldManager.h"
#include "GameCharacter.h"
#include "UnrealNetwork.h"

AShieldManager::AShieldManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AShieldManager::UpdateTotalShieldAmount()
{
	float localTotalShield = 0.f;

	for (auto& shield : shields)
		localTotalShield += shield.Value.amount;

	totalShieldAmount = localTotalShield;
}

void AShieldManager::AddShield(FCharacterShield newShield)
{
	if (!IsValid(owningCharacter))
		return;

	if (newShield.amountMax > 0)
	{
		newShield.amount = newShield.amountMax;
		shields.Add(newShield.key, newShield);

		if (newShield.duration > 0.f)
			owningCharacter->GetWorld()->GetTimerManager().SetTimer(newShield.timer, FTimerDelegate::CreateUObject(this, &AShieldManager::ShieldFinished, newShield), newShield.duration, false);

		UpdateTotalShieldAmount();
	}
}

bool AShieldManager::CanAbsorbDamage() const
{
	return GetTotalShieldAmount() > 0;
}

float AShieldManager::TryAbsorbDamage(float dmgAmount, TSubclassOf<UDamageType> dmgType)
{
	for (auto& shield : shields)
	{
		if (shield.Value.damageTypes.Contains(dmgType)) //found a shield that can absorb this type of damage
		{
			if (shield.Value.amount >= dmgAmount)
			{
				shield.Value.amount -= dmgAmount;
				dmgAmount = 0.f;
				break;
			}
			else
			{
				dmgAmount -= shield.Value.amount;
				shield.Value.amount = 0.f;
				ShieldFinished(shield.Value);
			}
		}
	}

	UpdateTotalShieldAmount();
	return dmgAmount;
}

float AShieldManager::GetTotalShieldAmount() const
{
	return totalShieldAmount;
}

void AShieldManager::ShieldFinished(FCharacterShield finishingShield)
{
	if (!IsValid(owningCharacter))
		return;

	owningCharacter->GetWorld()->GetTimerManager().ClearTimer(finishingShield.timer);
	shields.Remove(finishingShield.key);

	UpdateTotalShieldAmount();
}

void AShieldManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShieldManager, totalShieldAmount);
}