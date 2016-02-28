#include "Realm.h"
#include "GameCharacterData.h"
#include "StatsManager.h"

UGameCharacterData::UGameCharacterData(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

float* UGameCharacterData::GetCharacterBaseStats() const
{
	float* baseStats = new float[(uint8)EStat::ES_Max];

	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
		baseStats[i] = 0.f;

	baseStats[(uint8)EStat::ES_Atk] = attack;
	baseStats[(uint8)EStat::ES_Def] = defense;
	baseStats[(uint8)EStat::ES_AtkPL] = attackPerLevel;
	baseStats[(uint8)EStat::ES_DefPL] = defensePerLevel;
	baseStats[(uint8)EStat::ES_SpAtk] = specialAttack;
	baseStats[(uint8)EStat::ES_SpDef] = specialDefense;
	baseStats[(uint8)EStat::ES_SpAtkPL] = specialAttackPerLevel;
	baseStats[(uint8)EStat::ES_SpDefPL] = specialDefensePerLevel;
	baseStats[(uint8)EStat::ES_AtkSp] = attackSpeed;
	baseStats[(uint8)EStat::ES_AtkSpPL] = attackSpeedPerLevel;
	baseStats[(uint8)EStat::ES_Move] = movementSpeed;
	baseStats[(uint8)EStat::ES_HP] = health;
	baseStats[(uint8)EStat::ES_HPPL] = healthPerLevel;
	baseStats[(uint8)EStat::ES_HPRegen] = healthRegen;
	baseStats[(uint8)EStat::ES_HPRegenPL] = healthRegenPerLevel;
	baseStats[(uint8)EStat::ES_Flare] = flare;
	baseStats[(uint8)EStat::ES_FlarePL] = flarePerLevel;
	baseStats[(uint8)EStat::ES_FlareRegen] = flareRegen;
	baseStats[(uint8)EStat::ES_FlareRegenPL] = flareRegenPerLevel;

	return baseStats;
}