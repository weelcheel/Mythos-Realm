#pragma once

#include "Skill.h"
#include "SkillManager.generated.h"

class ASkill;

UCLASS()
class ASkillManager : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/* skills the character knows */
	UPROPERTY(replicated)
	TArray<ASkill*> skills;

public:

	/* add a skill to the skills array */
	void AddSkill(ASkill* newSkill);

	/* perform a skill */
	void ServerPerformSkill(int32 index, FVector mouseHitLoc, AGameCharacter* targetUnit);
	void ClientPerformSkill(int32 index, FVector mouseHitLoc, AGameCharacter* targetUnit);

	/* get a skill at an index */
	UFUNCTION(BlueprintCallable, Category=Skill)
	ASkill* GetSkill(int32 index);
};