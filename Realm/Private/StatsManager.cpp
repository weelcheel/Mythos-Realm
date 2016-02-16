#include "Realm.h"
#include "StatsManager.h"
#include "UnrealNetwork.h"
#include "Mod.h"
#include "GameCharacter.h"
#include "Effect.h"

AStatsManager::AStatsManager(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AStatsManager::SetMaxHealth()
{
	health = GetCurrentValueForStat(EStat::ES_HP);
}

void AStatsManager::SetMaxFlare()
{
	flare = GetCurrentValueForStat(EStat::ES_Flare);
}

void AStatsManager::InitializeStats(float* initBaseStats, AGameCharacter* ownerChar)
{
	if (bInitialized)
		return;

	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		baseStats[i] = initBaseStats[i];

	baseStats[(int32)EStat::ES_CritRatio] = 1.f;

	bInitialized = true;

	owningCharacter = ownerChar;
}

float AStatsManager::GetCurrentValueForStat(EStat stat) const
{
	if (stat == EStat::ES_AARange && bonusStats[(uint8)EStat::ES_AARange] > 0)
		stat = EStat::ES_AARange;

	return baseStats[(int32)stat] + modStats[(int32)stat] + bonusStats[(int32)stat];
}

float AStatsManager::GetBaseValueForStat(EStat stat) const
{
	return baseStats[(int32)stat];
}

float AStatsManager::GetUnaffectedValueForStat(EStat stat) const
{
	return baseStats[(int32)stat] + modStats[(int32)stat];
}

AEffect* AStatsManager::AddEffect(FText const& effectName, FText const& effectDescription, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration, FString const& keyName, bool bStacking, bool bMultipleInfliction)
{
	if (!IsValid(owningCharacter) || (IsValid(owningCharacter) && !owningCharacter->IsAlive())) //return if the character isnt valid or dead
		return nullptr;

	if (effectsMap.Contains(keyName) && !bMultipleInfliction) //return if this effect is already inflicted and can't be inflicted multiple times
		return nullptr;

	AEffect* newEffect = GetWorld()->SpawnActor<AEffect>(AEffect::StaticClass());

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

	if (Role == ROLE_Authority)
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
		GetWorldTimerManager().SetTimer(newEffect->effectTimer, FTimerDelegate::CreateUObject(this, &AStatsManager::EffectFinished, keyName), effectDuration, false);

	if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();

	return newEffect;
}

void AStatsManager::EffectFinished(FString key)
{
	if (!effectsMap.Contains(key))
		return;

	AEffect* effect = (*effectsMap.Find(key));

	if (!IsValid(effect))
		return;

	GetWorldTimerManager().ClearTimer(effect->effectTimer);

	if (Role == ROLE_Authority)
	{
		int32 ind = 0;
		for (TEnumAsByte<EStat> eStat : effect->stats)
		{
			bonusStats[(int32)eStat.GetValue()] -= effect->amounts[ind];
			ind++;
		}
	}

	effectsMap.Remove(key);
	effectsList.Remove(effect);

	effect->Destroy(true);

	if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
}

void AStatsManager::AddEffectStacks(const FString& effectKey, int32 stackAmount)
{
	if (effectsMap.Contains(effectKey))
	{
		effectsMap[effectKey]->stackAmount += stackAmount;

		if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
			owningCharacter->EffectsUpdated();
	}
}

AEffect* AStatsManager::GetEffect(const FString& effectKey)
{
	if (effectsMap.Contains(effectKey))
		return effectsMap[effectKey];
	else
		return nullptr;
}

void AStatsManager::RemoveHealth(float amount)
{
	health -= amount;
}

void AStatsManager::RemoveFlare(float amount)
{
	flare -= amount;
}

float AStatsManager::GetHealth() const
{
	return health;
}

float AStatsManager::GetFlare() const
{
	return flare;
}

void AStatsManager::UpdateModStats(TArray<AMod*>& mods)
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
				modStats[i] += GetCurrentValueForStat(EStat::ES_AtkSp) * mod->deltaStats[i];
			else
				modStats[i] += mod->deltaStats[i];
		}
	}
}

void AStatsManager::CharacterLevelUp()
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

void AStatsManager::OnRepUpdateEffects()
{
	//check for removed effects
	for (auto it = effectsMap.CreateIterator(); it; ++it)
	{
		bool bEffectFound = false;
		for (int32 i = 0; i < effectsList.Num(); i++)
			bEffectFound = effectsList[i]->GetName() == it.Key();

		if (!bEffectFound)
			effectsMap.Remove(it.Key());
	}

	//add new effects
	for (int32 i = 0; i < effectsList.Num(); i++)
	{
		AEffect* eff = effectsList[i];
		if (!IsValid(eff))
			continue;

		bool bEffectFound = false;
		for (auto it = effectsMap.CreateIterator(); it; ++it)
			bEffectFound = it.Key() == effectsList[i]->GetName();

		if (!bEffectFound)
			effectsMap.Add(effectsList[i]->GetName(), effectsList[i]);
	}

	if (IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
}

void AStatsManager::AddCreatedEffect(AEffect* newEffect)
{
	if (IsValid(newEffect))
	{
		if (effectsMap.Contains(newEffect->keyName) && !newEffect->bCanBeInflictedMultipleTimes)
			return;

		if (Role == ROLE_Authority)
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
			GetWorldTimerManager().SetTimer(newEffect->effectTimer, FTimerDelegate::CreateUObject(this, &AStatsManager::EffectFinished, newEffect->keyName), newEffect->duration, false);

		if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
			owningCharacter->EffectsUpdated();
	}
}

void AStatsManager::RemoveAllEffects()
{
	for (int32 i = 0; i < effectsList.Num(); i++)
	{
		if (IsValid(effectsList[i]))
			EffectFinished(effectsList[i]->keyName);
	}

	GetWorldTimerManager().ClearAllTimersForObject(this);

	effectsList.Empty();
	effectsMap.Empty(0);
}

void AStatsManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStatsManager, bInitialized);
	DOREPLIFETIME(AStatsManager, baseStats);
	DOREPLIFETIME(AStatsManager, modStats);
	DOREPLIFETIME(AStatsManager, bonusStats);
	DOREPLIFETIME(AStatsManager, health);
	DOREPLIFETIME(AStatsManager, flare);
	DOREPLIFETIME(AStatsManager, effectsList);
	DOREPLIFETIME(AStatsManager, owningCharacter);
}