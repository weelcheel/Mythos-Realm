#include "Realm.h"
#include "StatsManager.h"
#include "UnrealNetwork.h"
#include "Mod.h"

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

void AStatsManager::InitializeStats(float* initBaseStats)
{
	if (bInitialized)
		return;

	for (int32 i = 0; i < (uint8)EStat::ES_Max; i++)
		baseStats[i] = initBaseStats[i];

	bInitialized = true;
}

float AStatsManager::GetCurrentValueForStat(EStat stat) const
{
	return baseStats[(uint8)stat] + modStats[(uint8)stat] + bonusStats[(uint8)stat];
}

float AStatsManager::GetBaseValueForStat(EStat stat) const
{
	return baseStats[(uint8)stat];
}

float AStatsManager::GetUnaffectedValueForStat(EStat stat) const
{
	return baseStats[(uint8)stat] + modStats[(uint8)stat];
}

void AStatsManager::AddEffect(FString effectName, FString effectDescription, FString effectKey, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration /* = 0.f */, bool bStacking /*= false*/)
{
	for (int32 i = 0; i < effects.Num(); i++)
	{
		if (effects[i].effectKey == effectKey)
			return;
	}

	FEffect newEffect;

	newEffect.amounts = amounts;
	newEffect.stats = stats;
	newEffect.description = effectDescription;
	newEffect.uiName = effectName;
	newEffect.duration = effectDuration;
	newEffect.effectKey = effectKey;
	newEffect.bStacking = bStacking;
	newEffect.stackAmount = 0;

	if (Role == ROLE_Authority)
	{
		int32 ind = 0;
		for (TEnumAsByte<EStat> eStat : stats)
		{
			bonusStats[(uint8)eStat.GetValue()] += amounts[ind];
			ind++;
		}
	}

	int32 eInd = effects.Add(newEffect);

	if (effectDuration > 0.f)
		GetWorldTimerManager().SetTimer(effects[eInd].effectTimer, FTimerDelegate::CreateUObject(this, &AStatsManager::EffectFinished, newEffect.effectKey), effectDuration, false);
}

void AStatsManager::AddEffectStacks(const FString& effectKey, int32 stackAmount)
{
	for (int32 i = 0; i < effects.Num(); i++)
	{
		if (effectKey == effects[i].effectKey)
			effects[i].stackAmount += stackAmount;
	}
}

void AStatsManager::GetEffect(const FString& effectKey, FEffect& effect)
{
	for (int32 i = 0; i < effects.Num(); i++)
	{
		if (effectKey == effects[i].effectKey)
		{
			effect = effects[i];
			return;
		}
	}
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
	for (int32 i = 0; i < (uint8)EStat::ES_Max; i++)
		modStats[i] = 0.f;

	if (mods.Num() <= 0)
		return;

	for (AMod* mod : mods)
	{
		for (int32 i = 0; i < (uint8)EStat::ES_Max; i++)
			modStats[i] += mod->deltaStats[i];
	}
}

void AStatsManager::CharacterLevelUp()
{
	baseStats[(uint8)EStat::ES_HP] += GetCurrentValueForStat(EStat::ES_HPPL);
	health += GetCurrentValueForStat(EStat::ES_HPPL);
	baseStats[(uint8)EStat::ES_Flare] += GetCurrentValueForStat(EStat::ES_FlarePL);
	flare += GetCurrentValueForStat(EStat::ES_FlarePL);

	baseStats[(uint8)EStat::ES_HPRegen] += GetCurrentValueForStat(EStat::ES_HPRegenPL);
	baseStats[(uint8)EStat::ES_FlareRegen] += GetCurrentValueForStat(EStat::ES_FlareRegenPL);
	baseStats[(uint8)EStat::ES_SpAtk] += GetCurrentValueForStat(EStat::ES_SpAtkPL);
	baseStats[(uint8)EStat::ES_Atk] += GetCurrentValueForStat(EStat::ES_AtkPL);
	baseStats[(uint8)EStat::ES_Def] += GetCurrentValueForStat(EStat::ES_DefPL);
	baseStats[(uint8)EStat::ES_SpDef] += GetCurrentValueForStat(EStat::ES_SpDefPL);
	baseStats[(uint8)EStat::ES_AtkSp] += GetCurrentValueForStat(EStat::ES_AtkSpPL);
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
}