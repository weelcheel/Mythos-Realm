#include "Realm.h"
#include "StatsManager.h"
#include "UnrealNetwork.h"
#include "Mod.h"
#include "GameCharacter.h"

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

	for (int32 i = 0; i < (uint8)EStat::ES_Max; i++)
		baseStats[i] = initBaseStats[i];

	bInitialized = true;

	owningCharacter = ownerChar;
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

	effects.Add(effectKey, newEffect);

	if (effectDuration > 0.f)
		GetWorldTimerManager().SetTimer(effects[effectKey].effectTimer, FTimerDelegate::CreateUObject(this, &AStatsManager::EffectFinished, newEffect.effectKey), effectDuration, false);

	if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
}

void AStatsManager::EffectFinished(FString key)
{
	FEffect* effect = effects.Find(key);

	if (!effect)
		return;

	if (Role == ROLE_Authority)
	{
		int32 ind = 0;
		for (TEnumAsByte<EStat> eStat : effect->stats)
		{
			bonusStats[(uint8)eStat.GetValue()] -= effect->amounts[ind];
			ind++;
		}
	}

	effects.Remove(key);

	if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
}

void AStatsManager::AddEffectStacks(const FString& effectKey, int32 stackAmount)
{
	if (effects.Contains(effectKey))
	{
		effects[effectKey].stackAmount += stackAmount;

		if ((GetWorld()->GetNetMode() == NM_Standalone || GetWorld()->GetNetMode() == NM_ListenServer) && IsValid(owningCharacter))
			owningCharacter->EffectsUpdated();
	}
}

void AStatsManager::GetEffect(const FString& effectKey, FEffect& effect)
{
	FEffect invalid;

	invalid.effectKey = "invalid";

	if (effects.Contains(effectKey))
		effect = effects[effectKey];
	else
		effect = invalid;
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

void AStatsManager::OnRepUpdateEffects()
{
	if (IsValid(owningCharacter))
		owningCharacter->EffectsUpdated();
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
	DOREPLIFETIME(AStatsManager, effects);
	DOREPLIFETIME(AStatsManager, owningCharacter);
}