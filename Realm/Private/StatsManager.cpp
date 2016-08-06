#include "Realm.h"
#include "StatsManager.h"
#include "UnrealNetwork.h"
#include "Mod.h"
#include "GameCharacter.h"
#include "Effect.h"

UStatsManager::UStatsManager(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		bonusStats[i] = 0.f;
}

void UStatsManager::SetMaxHealth()
{
	health = GetCurrentValueForStat(EStat::ES_HP) + bonusStats[(uint16)EStat::ES_HP];
}

void UStatsManager::SetMaxFlare()
{
	flare = GetCurrentValueForStat(EStat::ES_Flare);
}

void UStatsManager::InitializeStats(float* initBaseStats, AGameCharacter* ownerChar)
{
	if (bInitialized)
		return;

	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		baseStats[i] = initBaseStats[i];

	baseStats[(int32)EStat::ES_CritRatio] = 100.f;

	bInitialized = true;

	owningCharacter = ownerChar;
}

float UStatsManager::GetCurrentValueForStat(EStat stat) const
{
	if (stat == EStat::ES_AARange && bonusStats[(uint8)EStat::ES_AARange] > 0)
		stat = EStat::ES_AARange;

	return baseStats[(int32)stat] + modStats[(int32)stat] + bonusStats[(int32)stat];
}

float UStatsManager::GetBaseValueForStat(EStat stat) const
{
	return baseStats[(int32)stat];
}

float UStatsManager::GetUnaffectedValueForStat(EStat stat) const
{
	return baseStats[(int32)stat] + modStats[(int32)stat];
}

AEffect* UStatsManager::AddEffect(FText const& effectName, FText const& effectDescription, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration, FString const& keyName, bool bStacking, bool bMultipleInfliction, bool bPersistThroughDeath, UParticleSystem* effectParticle)
{
	if (!IsValid(owningCharacter) || (IsValid(owningCharacter) && !owningCharacter->IsAlive())) //return if the character isnt valid or dead
		return nullptr;

	if (effectsMap.Contains(keyName) && !bMultipleInfliction) //return if this effect is already inflicted and can't be inflicted multiple times
		return nullptr;

	AEffect* newEffect = owningCharacter->GetWorld()->SpawnActor<AEffect>(AEffect::StaticClass());

	newEffect->amounts = amounts;
	newEffect->stats = stats;
	newEffect->description = effectDescription;
	newEffect->uiName = effectName;
	newEffect->duration = effectDuration;
	newEffect->bStacking = bStacking;
	newEffect->stackAmount = 0;
	newEffect->keyName = keyName;
	newEffect->bCanBeInflictedMultipleTimes = bMultipleInfliction;
	newEffect->statsManager = this;
	newEffect->bPersistThroughDeath = bPersistThroughDeath;
	newEffect->effectParticle = effectParticle;

	newEffect->currentEmitter = UGameplayStatics::SpawnEmitterAttached(effectParticle, owningCharacter->GetRootComponent());

	if (owningCharacter->HasAuthority())
	{
		int32 ind = 0;
		for (TEnumAsByte<EStat> eStat : stats)
		{
			bonusStats[(int32)eStat.GetValue()] += amounts[ind];
			ind++;
		}
	}

	effectsMap.Add(keyName, newEffect);
	effectsList.AddUnique(newEffect);

	if (effectDuration > 0.f)
		owningCharacter->GetWorldTimerManager().SetTimer(newEffect->effectTimer, FTimerDelegate::CreateUObject(this, &UStatsManager::EffectFinished, keyName), effectDuration, false);

	if (IsValid(owningCharacter) && (owningCharacter->GetWorld()->GetNetMode() == NM_Standalone || owningCharacter->GetWorld()->GetNetMode() == NM_ListenServer))
		owningCharacter->EffectsUpdated();

	return newEffect;
}

void UStatsManager::EffectFinished(FString key)
{
	if (!effectsMap.Contains(key) || !IsValid(owningCharacter))
		return;

	AEffect* effect = (*effectsMap.Find(key));

	if (!IsValid(effect))
		return;

	owningCharacter->GetWorldTimerManager().ClearTimer(effect->effectTimer);

	if (owningCharacter->HasAuthority())
	{
		int32 ind = 0;
		for (TEnumAsByte<EStat> eStat : effect->stats)
		{
			bonusStats[(int32)eStat.GetValue()] -= effect->amounts[ind];
			ind++;
		}
	}

	if (IsValid(effect->currentEmitter))
	{
		effect->currentEmitter->DeactivateSystem();
		effect->currentEmitter->DestroyComponent();
	}
		

	effectsMap.Remove(key);
	effectsList.Remove(effect);

	effect->SetLifeSpan(0.05f);

	if (IsValid(owningCharacter) && (owningCharacter->GetWorld()->GetNetMode() == NM_Standalone || owningCharacter->GetWorld()->GetNetMode() == NM_ListenServer))
		owningCharacter->EffectsUpdated();
}

void UStatsManager::AddEffectStacks(const FString& effectKey, int32 stackAmount)
{
	if (!IsValid(owningCharacter))
		return;

	if (effectsMap.Contains(effectKey))
	{
		effectsMap[effectKey]->stackAmount += stackAmount;

		if ((owningCharacter->GetWorld()->GetNetMode() == NM_Standalone || owningCharacter->GetWorld()->GetNetMode() == NM_ListenServer))
			owningCharacter->EffectsUpdated();
	}
}

AEffect* UStatsManager::GetEffect(const FString& effectKey)
{
	if (effectsMap.Contains(effectKey))
		return effectsMap[effectKey];
	else
		return nullptr;
}

void UStatsManager::RemoveHealth(float amount)
{
	health -= amount;
}

void UStatsManager::RemoveFlare(float amount)
{
	flare -= amount;
}

float UStatsManager::GetHealth() const
{
	return health;
}

float UStatsManager::GetFlare() const
{
	return flare;
}

void UStatsManager::UpdateModStats(TArray<AMod*>& mods)
{
	//clear the mods array
	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		modStats[i] = 0.f;

	if (mods.Num() <= 0)
		return;

	for (AMod* mod : mods)
	{
		for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		{
			if (i == (int32)EStat::ES_AtkSp)
				modStats[i] += (GetCurrentValueForStat(EStat::ES_AtkSp) / 100.f) * mod->deltaStats[i];
			else
				modStats[i] += mod->deltaStats[i];
		}
	}
}

void UStatsManager::CharacterLevelUp()
{
	baseStats[(int32)EStat::ES_HP] += GetCurrentValueForStat(EStat::ES_HPPL);
	health += GetCurrentValueForStat(EStat::ES_HPPL);
	baseStats[(int32)EStat::ES_Flare] += GetCurrentValueForStat(EStat::ES_FlarePL);
	flare += GetCurrentValueForStat(EStat::ES_FlarePL);

	baseStats[(int32)EStat::ES_HPRegen] += GetCurrentValueForStat(EStat::ES_HPRegenPL);
	baseStats[(int32)EStat::ES_FlareRegen] += GetCurrentValueForStat(EStat::ES_FlareRegenPL);
	baseStats[(int32)EStat::ES_SpAtk] += GetCurrentValueForStat(EStat::ES_SpAtkPL);
	baseStats[(int32)EStat::ES_Atk] += GetCurrentValueForStat(EStat::ES_AtkPL);
	baseStats[(int32)EStat::ES_Def] += GetCurrentValueForStat(EStat::ES_DefPL);
	baseStats[(int32)EStat::ES_SpDef] += GetCurrentValueForStat(EStat::ES_SpDefPL);
	baseStats[(int32)EStat::ES_AtkSp] += GetCurrentValueForStat(EStat::ES_AtkSpPL);
}

void UStatsManager::OnRepUpdateEffects()
{
	//check for removed effects
	for (auto it = effectsMap.CreateIterator(); it; ++it)
	{
		AEffect* hashEffect = it.Value();

		if (!IsValid(hashEffect))
			continue;

		bool bFoundEffect = false;
		for (int32 i = 0; i < effectsList.Num(); i++)
		{
			bFoundEffect = effectsList[i]->GetName() == hashEffect->GetName();
			if (bFoundEffect)
				break;
		}

		if (!bFoundEffect)
		{
			if (IsValid(hashEffect->currentEmitter))
			{
				hashEffect->currentEmitter->DeactivateSystem();
				hashEffect->currentEmitter->DestroyComponent();
			}

			hashEffect->Destroy(true);
			effectsMap.Remove(it.Key());
		}
	}

	//add new effects
	for (int32 i = 0; i < effectsList.Num(); i++)
	{
		AEffect* eff = effectsList[i];
		if (!IsValid(eff))
			continue;

		if (!effectsMap.Contains(eff->GetName()))
		{
			if (IsValid(eff->effectParticle))
				eff->currentEmitter = UGameplayStatics::SpawnEmitterAttached(eff->effectParticle, owningCharacter->GetRootComponent());
			effectsMap.Add(eff->GetName(), eff);
		}
	}

	if (IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
}

void UStatsManager::AddCreatedEffect(AEffect* newEffect)
{
	if (IsValid(newEffect))
	{
		if (effectsMap.Contains(newEffect->keyName) && !newEffect->bCanBeInflictedMultipleTimes)
			return;

		if (owningCharacter->HasAuthority())
		{
			int32 ind = 0;
			for (TEnumAsByte<EStat> eStat : newEffect->stats)
			{
				bonusStats[(int32)eStat.GetValue()] += newEffect->amounts[ind];
				ind++;
			}
		}

		effectsList.AddUnique(newEffect);
		effectsMap.Add(newEffect->keyName, newEffect);

		if (newEffect->duration > 0.f)
			owningCharacter->GetWorldTimerManager().SetTimer(newEffect->effectTimer, FTimerDelegate::CreateUObject(this, &UStatsManager::EffectFinished, newEffect->keyName), newEffect->duration, false);

		if (IsValid(owningCharacter) && (owningCharacter->GetWorld()->GetNetMode() == NM_Standalone || owningCharacter->GetWorld()->GetNetMode() == NM_ListenServer))
			owningCharacter->EffectsUpdated();
	}
}

void UStatsManager::RemoveAllEffects(bool bFromDeath)
{
	for (int32 i = 0; i < effectsList.Num(); i++)
	{
		if (bFromDeath)
		{
			if (IsValid(effectsList[i]) && !effectsList[i]->bPersistThroughDeath)
				EffectFinished(effectsList[i]->keyName);
		}
		else
		{
			if (IsValid(effectsList[i]))
				EffectFinished(effectsList[i]->keyName);

			effectsList.Empty();
			effectsMap.Empty(0);
		}
	}

	if (IsValid(owningCharacter))
		owningCharacter->GetWorldTimerManager().ClearAllTimersForObject(this);
}

void UStatsManager::RemoveNegativeEffects()
{
	for (int32 i = 0; i < effectsList.Num(); i++)
	{
		if (IsValid(effectsList[i]))
		{
			for (float amt : effectsList[i]->amounts)
			{
				if (amt < 0.f)
					EffectFinished(effectsList[i]->keyName);
			}
		}
	}
}

void UStatsManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	DOREPLIFETIME(UStatsManager, bInitialized);
	DOREPLIFETIME(UStatsManager, baseStats);
	DOREPLIFETIME(UStatsManager, modStats);
	DOREPLIFETIME(UStatsManager, bonusStats);
	DOREPLIFETIME(UStatsManager, health);
	DOREPLIFETIME(UStatsManager, flare);
	DOREPLIFETIME(UStatsManager, effectsList);
	DOREPLIFETIME(UStatsManager, owningCharacter);
}