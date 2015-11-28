#include "Realm.h"
#include "SkillManager.h"
#include "UnrealNetwork.h"
#include "Skill.h"
#include "GameCharacter.h"

ASkillManager::ASkillManager(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ASkillManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASkillManager, skills);
}

void ASkillManager::AddSkill(ASkill* newSkill)
{
	skills.AddUnique(newSkill);
}

void ASkillManager::ServerPerformSkill(int32 index, FVector mouseHitLoc, AGameCharacter* targetUnit)
{
	if (index < skills.Num())
		skills[index]->ServerSkillPerformed(mouseHitLoc, targetUnit);
}

void ASkillManager::ClientPerformSkill(int32 index, FVector mouseHitLoc, AGameCharacter* targetUnit)
{
	if (index < skills.Num())
		skills[index]->ClientSkillPerformed(mouseHitLoc, targetUnit);
}

ASkill* ASkillManager::GetSkill(int32 index)
{
	if (index < skills.Num())
		return skills[index];
	else
		return nullptr;
}