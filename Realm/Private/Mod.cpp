#include "Realm.h"
#include "Mod.h"

AMod::AMod(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

FString AMod::GetStatsDescription() const
{
	FString statsDesc = "";

	for (int32 i = 0; i < (int32)EStat::ES_Max; i++)
	{
		if (deltaStats[i] != 0.f)
		{
			switch (i)
			{
			case EStat::ES_Atk:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Attack";
				break;
			case EStat::ES_Def:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Defense";
				break;
			case EStat::ES_AtkPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Attack per Level";
				break;
			case EStat::ES_DefPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Defense per Level";
				break;
			case EStat::ES_SpAtk:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Special Attack";
				break;
			case EStat::ES_SpDef:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Special Defense";
				break;
			case EStat::ES_SpAtkPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Special Attack per Level";
				break;
			case EStat::ES_SpDefPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Special Defense per Level";
				break;
			case EStat::ES_AARange:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Auto Attack Range";
				break;
			case EStat::ES_AtkSp:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i] * 100.f)) + "% Attack Speed";
				break;
			case EStat::ES_AtkSpPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + "% Attack Speed per Level";
				break;
			case EStat::ES_Move:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Movement Speed";
				break;
			case EStat::ES_CDR:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i] * 100.f)) + "% Cooldown Reduction";
				break;
			case EStat::ES_HP:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Health";
				break;
			case EStat::ES_HPPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Health per Level";
				break;
			case EStat::ES_HPRegen:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Health Regen per Second";
				break;
			case EStat::ES_HPRegenPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Health Regen per Second per Level";
				break;
			case EStat::ES_Flare:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i])) + " Flare";
				break;
			case EStat::ES_FlarePL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Flare per Level";
				break;
			case EStat::ES_FlareRegen:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Flare Regen per Second";
				break;
			case EStat::ES_FlareRegenPL:
				statsDesc += "+" + FString::SanitizeFloat(deltaStats[i]) + " Flare Regen per Second per Level";
				break;
			case EStat::ES_CritChance:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i] * 100.f)) + "% Critical Hit Chance";
				break;
			case EStat::ES_CritRatio:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i] * 100.f)) + "% Critical Hit Damage";
				break;
			case EStat::ES_HPDrain:
				statsDesc += "+" + FString::FromInt(FMath::RoundToInt(deltaStats[i] * 100.f)) + "% Health Drain";
				break;
			}

			statsDesc += "\n";
		}
	}

	return statsDesc;
}

AMod* AMod::GetDefaultModObject(TSubclassOf<AMod> modClass)
{
	return modClass->GetDefaultObject<AMod>();
}