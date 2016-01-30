#pragma once

#include "Skill.generated.h"

class AGameCharacter;

UENUM()
enum class ESkillState : uint8
{
	NoOwner,
	Ready,
	NotLearned,
	Disabled,
	OnCooldown,
	Performing,
	MAX
};

UCLASS()
class ASkill : public AActor
{
	friend class AGameCharacter;

	GENERATED_UCLASS_BODY()

protected:

	/* what current state the skill is in */
	UPROPERTY(replicated)
	TEnumAsByte<ESkillState> skillState;

	/* character using this skill */
	UPROPERTY(BlueprintReadWrite, replicated, Category = Character)
	AGameCharacter* characterOwner;

	/* amount of skill points this skill has */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Skill)
	int32 skillPoints;

	/* max amount of skill points the skill can have */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Skill)
	int32 skillPointsMax;

	/* timer for cooldowns */
	FTimerHandle cooldownTimer;

	/* for when the cooldown timer is set */
	UPROPERTY(ReplicatedUsing = OnCooldownTimerSet)
	float cooldownTime;

	/* whether or not this skill automatically enters the performing state on use */
	UPROPERTY(EditDefaultsOnly, Category = Skill)
	bool bAutoPerform;

	/* minimum amount of time for cooldown for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cooldownMin;

	/* max amount of time for cooldown for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cooldownMax;

	/* minimum amount of cost for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cost;

	/* name of this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	FText skillName;

	/* called when cooldown is finished */
	void CooldownFinished();

	UFUNCTION()
	void OnCooldownTimerSet();

public:

	/* sphere trace with a certain radius */
	UFUNCTION(BlueprintCallable, Category = Trace)
	static bool SphereTrace(AActor* actorToIgnore, const FVector& start, const FVector& end, const float radius, TArray<FHitResult>& hitOut, ECollisionChannel traceChannel = ECC_Pawn);

	/*since there's no native cone collision detection, cheat by creating a set of increasing radius spheres in a line */
	UFUNCTION(BlueprintCallable, Category = Trace)
	static bool ConeTrace(AActor* actorToIgnore, const FVector& start, const FVector& dir, float coneHeight, TArray<AGameCharacter*>& hitsOut, ECollisionChannel traceChannel = ECC_Pawn);

	/* initialize this skill to a character specified */
	void InitializeSkill(AGameCharacter* owner);

	/* [CLIENT] skill specific logic that happens on every client */
	UFUNCTION(BlueprintImplementableEvent, Category = Skill)
		void ClientSkillPerformed(FVector mouseHitLoc, AGameCharacter* targetUnit = NULL);

	/* [SERVER] skill specific logic that happens when this skill is activated on the server side (important logic here) */
	UFUNCTION(BlueprintNativeEvent, Category = Skill)
		void ServerSkillPerformed(FVector mouseHitLoc, AGameCharacter* targetUnit = NULL);

	/* start cooldown for the skill */
	UFUNCTION(BlueprintCallable, Category = Skill)
		void StartCooldown();

	/* [SERVER] add a skill point */
	UFUNCTION(BlueprintCallable, Category = Skill)
		void AddSkillPoint();

	/* gets a value scaled by this skill's level */
	UFUNCTION(BlueprintCallable, Category = Skill)
		float SkillLevelScale(float min, float max, bool bIncreasing) const;

	/* creates a damage event struct */
	UFUNCTION(BlueprintCallable, Category = Damage)
		static FDamageEvent CreateDamageEvent(TSubclassOf<UDamageType> damageType);

	/* gets the percentage of cooldown progress */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCooldownProgressPercent();

	/* gets the amount of time left in the cooldown */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCooldownRemaining();

	/* gets the current cost of this skill */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCost();

	/* called after the skill is finished being performed to use flare and start cooldowns */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
	void SkillFinished();

	/* gets the current skill state */
	ESkillState GetSkillState() const;

	/* sets the current skill state */
	UFUNCTION(BlueprintCallable, Category=Skill)
	void SetSkillState(ESkillState newState);
};