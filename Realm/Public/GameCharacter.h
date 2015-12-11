#pragma once

#include "RealmCharacter.h"
#include "AutoAttackManager.h"
#include "SkillManager.h"
#include "DamageTypes.h"
#include "DamageInstance.h"
#include "Mod.h"
#include "GameCharacter.generated.h"

/* max level for characters */
const static int32 MAX_LEVEL = 15;
const static float EXP_CONST = 2.f / FMath::Sqrt(128.f);

UCLASS(ABSTRACT, Blueprintable)
class AGameCharacter : public ARealmCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* base stats to initialize the stats manager with */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float baseStats[(uint8)EStat::ES_Max];

	/* stat manager for handling stat updates */
	UPROPERTY(replicated)
	AStatsManager* statsManager;

	/* auto attack manager this character can use */
	UPROPERTY(replicated)
	AAutoAttackManager* autoAttackManager;

	/* skill manager this character can use */
	UPROPERTY(replicated)
	ASkillManager* skillManager;

	/* array of auto attacks this character can use */
	UPROPERTY(EditDefaultsOnly, Category = AA)
	TArray<FAutoAttack> autoAttacks;

	/* array of classes of skills the character uses */
	UPROPERTY(EditDefaultsOnly, Category = Skill)
	TArray<TSubclassOf<ASkill> > skillClasses;

	/* index of team this character is on. this will be overridden if the character is owned by a player */
	UPROPERTY(replicated, EditAnywhere, Category = Team)
	int32 teamIndex;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	FTakeHitInfo lastTakeHitInfo;

	/* array of mods this actor currently has */
	UPROPERTY(replicated, VisibleAnywhere, Category = Mods)
	TArray<AMod*> mods;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health)
	uint32 bIsDying : 1;

	/* if this unit's auto attack is on cooldown */
	UPROPERTY(BlueprintReadOnly, Category = AA)
	bool bAutoAttackOnCooldown;

	/* if this unit's auto attack is being launched */
	UPROPERTY(BlueprintReadOnly, Category = AA)
	bool bAutoAttackLaunching;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float lastTakeHitTimeTimeout;

	/* amount of experience this character has */
	UPROPERTY(replicated, BlueprintReadOnly, Category=Exp)
	int32 experienceAmount;

	/* what level this character currently is */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Exp)
	int32 level;

	/* amount of skill points this character has to put on skills */
	UPROPERTY(BlueprintReadOnly, Category = Exp)
	int32 skillPoints;

	/*base amount of experience that this character is worth when killed */
	UPROPERTY(EditDefaultsOnly, Category = Exp)
	int32 baseExpReward;

	/* range that this character gives off experience */
	UPROPERTY(EditDefaultsOnly, Category = Exp)
	float experienceRewardRange;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** sets up the replication for taking a hit */
	virtual void ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled);

	/** play hit or death on client */
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	/* regen functions */
	void HealthRegen();
	void FlareRegen();

	/** cleanup inventory */
	virtual void Destroy(bool bNetForce  = false, bool bShouldModifyLevel = true);

	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/* perform level up */
	void LevelUp();

public:

	/* timers for auto attacks */
	FTimerHandle aaRangeTimer, aaTimer, aaLaunchTimer;

	/* timer for actions */
	FTimerHandle actionTimer, respawnTimer;
	FTimerHandle healthRegen, flareRegen;

	/* current target for this character */
	AGameCharacter* currentTarget;

	/* perform the server version of the skill then perform the skill on all clients */
	UFUNCTION(NetMulticast, reliable, WithValidation)
	void UseSkill(int32 index, FVector mouseHitLoc, AGameCharacter* unitTarget);

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class AGameCharacter* PawnInstigator, class AActor* DamageCauser);

	/* [SERVER] launch the charcater's auto attack once in range */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void StartAutoAttack();

	/* actually launch the attack (this is the point where the attack cannot be cancelled) */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void LaunchAutoAttack();

	/* try to cancel any auto attacks in progress of being performed and clear current target */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void StopAutoAttack();

	/* this is called every 20th while the auto attack is launched to make sure were still in range */
	virtual void CheckAutoAttack();

	/* stop auto attack cooldown */
	void OnFinishAATimer();

	/* get the current target for this character */
	UFUNCTION(BlueprintCallable, Category = Target)
	AGameCharacter* GetCurrentTarget() const;

	/* set the current target */
	UFUNCTION(BlueprintCallable, Category = Target)
	void SetCurrentTarget(AGameCharacter* newTarget);

	/* get the stat manager */
	UFUNCTION(BlueprintCallable, Category = Stats)
	AStatsManager* GetStatsManager() const;

	/* get the auto attack manager */
	UFUNCTION(BlueprintCallable, Category = AA)
	AAutoAttackManager* GetAutoAttackManager() const;

	/* get the skill manager */
	UFUNCTION(BlueprintCallable, Category = Stats)
	ASkillManager* GetSkillManager() const;

	/* gets the current value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetCurrentValueForStat(EStat stat) const;

	/* gets the base value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetBaseValueForStat(EStat stat) const;

	/* get the unaffected value of the specified stat */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetUnaffectedValueForStat(EStat stat) const;

	/* add buff/debuff to the player's stats */
	UFUNCTION(NetMulticast, reliable, WithValidation, BlueprintCallable, Category = Stat)
	void AddEffect(const FString& effectName, const FString& effectDescription, const FString& effectKey, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration = 0.f, bool bStacking = false);

	UFUNCTION(NetMulticast, reliable, WithValidation, BlueprintCallable, Category = Stat)
	void AddEffectStacks( const FString& effectKey,  int32 stackAmount);

	UFUNCTION(NetMulticast, reliable, WithValidation, BlueprintCallable, Category = Stat)
	void EndEffect(const FString& effectKey);

	/** Take damage, handle death */
	UFUNCTION(BlueprintCallable, Category=Damage)
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/* get the health for this character */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetHealth() const;

	/* get the health for this character */
	UFUNCTION(BlueprintCallable, Category = Stat)
	float GetFlare() const;

	/* use flare for this character */
	UFUNCTION(BlueprintCallable, Category = Stat)
	void UseFlare(float amount);

	/* get the team index of this character */
	UFUNCTION(BlueprintCallable, Category = Team)
	int32 GetTeamIndex() const;

	/* set the team index of this character */
	UFUNCTION(BlueprintCallable, Category = Team)
	void SetTeamIndex(int32 newTeam);

	/** check if pawn is still alive */
	bool IsAlive() const;

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, APawn* Killer, AActor* DamageCauser) const;

	/* let the specific classes have different character overlays */
	virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	/* killed by */
	virtual void KilledBy(APawn* EventInstigator);

	/** Pawn suicide */
	virtual void Suicide();

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* Killer, class AActor* DamageCauser);

	/** switch to ragdoll */
	void SetRagdollPhysics();

	/* stop ragdoll */
	void StopRagdollPhysics();

	/* adds a mod to the character and updates stats */
	UFUNCTION(BlueprintCallable, Category = Mods)
	void AddMod(AMod* newMod);

	/* removes a mod from character and updates stats */
	UFUNCTION(BlueprintCallable, Category = Mods)
	void RemoveMod(int32 index);

	/* get mod count */
	UFUNCTION(BlueprintCallable, Category = Mods)
	int32 GetModCount();

	/* Receive a distress call from a friendly unit */
	virtual void ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget);

	/* gets the relevant controller for this character, either the AI controller or the player controller if there is one */
	UFUNCTION(BlueprintCallable, Category = Controller)
	AController* GetRealmController() const;

	/* tries to perform a character dash in a given velocity */
	UFUNCTION(BlueprintCallable, Category = Dash)
	void CharacterDash(FVector dashVelocity);

	/* tries to end a character dash */
	UFUNCTION(BlueprintCallable, Category = Dash)
	void EndCharacterDash();

	/* blueprint hook for when the dash begins */
	UFUNCTION(BlueprintImplementableEvent, Category = Dash)
	void CharacterDashStarted();

	/* blueprint hook for when the dash is finished */
	UFUNCTION(BlueprintImplementableEvent, Category = Dash)
	void CharacterDashFinished();

	/* blueprint event caller for when this character takes damage */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void CharacterDamaged(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AGameCharacter* dmgCauser, AActor* actorCauser);

	/* blueprint hook for when this character damages another */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void DamagedOtherCharacter(AGameCharacter* hitCharacter, int32 damageAmount = 0, bool bAutoAttack = false);

	/* get the amount of experience needed for the next level */
	UFUNCTION(BlueprintCallable, Category = Exp)
	int32 GetNextLevelExperience() const;

	/* give this character experience */
	UFUNCTION(BlueprintCallable, Category = Exp)
	virtual void GiveCharacterExperience(int32 amount);

	/* blueprint hook for when effects change or update (for HUDs) */
	void EffectsUpdated();
};