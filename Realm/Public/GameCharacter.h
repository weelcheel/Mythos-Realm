#pragma once

#include "RealmCharacter.h"
#include "AutoAttackManager.h"
#include "SkillManager.h"
#include "DamageTypes.h"
#include "DamageInstance.h"
#include "ModManager.h"
#include "Mod.h"
#include "GameCharacterData.h"
#include "ShieldManager.h"
#include "GameCharacter.generated.h"

/* max level for characters */
const static int32 MAX_LEVEL = 15;
const static float EXP_CONST = 2.f / FMath::Sqrt(128.f);

class URealmFogofWarManager;
class UOverheadWidget;
class UUserWidget;
class AStealthArea;

/* types for hard Crowd Control (Ailments) */
UENUM(BlueprintType)
enum class EAilment : uint8
{
	AL_None UMETA(DisplayName = "No Ailment"),
	AL_Knockup UMETA(DisplayName = "Knocked Up"),
	AL_Stun UMETA(DisplayName = "Stunned"),
	AL_Neutral UMETA(DisplayName = "Neutralized"),
	AL_Blind UMETA(DisplayName = "Blinded"),
	AL_Max UMETA(Hidden)
};

/* struct for holding all of the data that an ailment needs */
USTRUCT(BlueprintType)
struct FAilmentInfo
{
	GENERATED_USTRUCT_BODY()

	/* type of ailment this is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ailment)
	EAilment newAilment;

	/* text to represent the ailment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ailment)
	FString ailmentText;

	/* duration this ailment normally lasts for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ailment)
	float ailmentDuration;

	/* any directional and magnitude info associated with the ailment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ailment)
	FVector ailmentDir;
};

/* struct for holding damage over time info */
USTRUCT()
struct FDamageOverTime
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	float tickDamageTotal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	float tickDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	float tickInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	float dotDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	float incurredTickDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	struct FDamageEvent DamageEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	class AController* EventInstigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	class AActor* DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	FRealmDamage realmDamage; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	FTimerHandle dotTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoT)
	FDamageRecap damageDesc;
};

UCLASS(ABSTRACT, Blueprintable)
class AGameCharacter : public ARealmCharacter
{
	friend class ARealmGameMode;
	friend class ARealmPlayerController;
	friend class URealmCharacterMovementComponent;
	friend class URealmFogofWarManager;

	GENERATED_UCLASS_BODY()

protected:

	/* character data (so we can have variants of the same character, like different character skins for example) */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	TSubclassOf<UGameCharacterData> characterData;

	/* current target for this character */
	UPROPERTY(replicated)
	AGameCharacter* currentTarget;

	/* stat manager for handling stat updates */
	UPROPERTY(replicated)
	UStatsManager* statsManager;

	/* auto attack manager this character can use */
	UPROPERTY(replicated)
	UAutoAttackManager* autoAttackManager;

	/* skill manager this character can use */
	UPROPERTY(replicated)
	ASkillManager* skillManager;

	/* mod manager this character can use */
	UPROPERTY(replicated)
	UModManager* modManager;
	
	/* shield manager this character can use */
	UPROPERTY(replicated, BlueprintReadOnly, Category=Shield)
	AShieldManager* shieldManager;

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
	UPROPERTY(BlueprintReadOnly, Category = AA, ReplicatedUsing=OnRep_AutoAttackLaunching)
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
	UPROPERTY(BlueprintReadOnly, replicated, Category = Exp)
	int32 skillPoints;

	/*base amount of experience that this character is worth when killed */
	UPROPERTY(EditDefaultsOnly, Category = Exp)
	int32 baseExpReward;

	/* range that this character gives off experience */
	UPROPERTY(EditDefaultsOnly, Category = Exp)
	float experienceRewardRange;

	/* current ailment (if any) affecting this character */
	UPROPERTY(ReplicatedUsing = OnRepAilment)
	FAilmentInfo currentAilment;

	/* array of ailments that need to be inflicted to the character (if there is an ailment currently being processed) */
	TQueue<FAilmentInfo> ailmentQueue;

	/* timer for handling aimlments */
	UPROPERTY(BlueprintReadOnly, Category=Ailment)
	FTimerHandle ailmentTimer;

	/* name of the action this character is currently performing */
	UPROPERTY(BlueprintReadOnly, Category = Action)
	FString currentActionName;

	/* whether or not this action has a reversed progress bar */
	UPROPERTY(BlueprintReadOnly, Category = Action)
	bool bReverseActionBar;

	/* whether or not the current action is preventing this unit from moving */
	bool bActionPreventingMovement = false;

	/* whether or not this character is seen by the enemy */
	UPROPERTY(BlueprintReadOnly, Category = Sight)
	bool bCanEnemySee = false;

	/* whether or not his character is guaranteed to crit next hit */
	bool bGuaranteeCrit = false;

	/* whether or not this character is in combat */
	UPROPERTY(BlueprintReadOnly, Category=Combat)
	bool bInCombat = false;

	/* whether or not this character should negate the next damage event */
	bool bNegateNextDmgEvent = false;

	/* period of time this character needs to take no damage or perform combat actions to leave combat */
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float combatTimeoutDelay;
	
	/* whether or not this character is targetable */
	UPROPERTY(replicated)
	bool bIsTargetable = true;

	/* whether or not this character can only be damaged by specific characters */
	UPROPERTY()
	bool bOnlySpecificCharactersCanDamage = false;

	/* whether or not this unit is accepting move commands right now */
	UPROPERTY()
	bool bAcceptingMoveCommands = true;

	/* list of characters that can damage us in specific damage mode */
	UPROPERTY()
	TArray<AGameCharacter*> specificDamagingCharacters;

	/* timer for this character's combat phase */
	FTimerHandle combatTimeout;

	/* text for the name of this character to show in-game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CharacterName)
	FText characterName;

	/* sound attenuation to use for character sounds */
	UPROPERTY()
	USoundAttenuation* soundAttenuation;

	/* class of UI widget to display above the character's head */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UOverheadWidget> overheadWidgetClass;

	/* UI widget to display above this character's head for UI info about them */
	UPROPERTY()
	UOverheadWidget* overheadWidget;

	/* UI widget to display for this character on the minimap. Should probably make this base class in C++ eventually */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	TSubclassOf<UUserWidget> minimapIconClass;

	/* particle system to play when this character receives damage from an auto attack */
	UPROPERTY(EditDefaultsOnly, Category = Particles)
	UParticleSystem* aaDamagedParticleSystem;

	/* how many times the character's half height to place the overhead widget over head */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	float overheadHalfHeightMultiplier;

	/* whether or not the current action being performed by this unit prevents combat */
	bool bActionPreventingCombat = false;

	/* whether or not this character is being controlled by another */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	AGameCharacter* controllingCharacter;

	/* function called on server when the current action has finished */
	void CharacterActionFinished();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** sets up the replication for taking a hit */
	virtual void ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled, FRealmDamage& realmDamage, FDamageRecap& damageDesc);

	/** play hit or death on client */
	UFUNCTION()
	virtual void OnRep_LastTakeHitInfo();

	/* notify the client of Ailment */
	UFUNCTION()
	virtual void OnRepAilment();

	/* notify of starting to launch auto attack */
	UFUNCTION()
	void OnRep_AutoAttackLaunching();

	/* regen functions */
	void HealthRegen();
	void FlareRegen();

	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc);

	/* perform level up */
	void LevelUp();

	/* called whenever this character damages another */
	void HurtAnother(AGameCharacter* hurtCharacter, struct FDamageEvent const& DamageEvent, float damageAmount = 0.f, FRealmDamage const& realmDamage = FRealmDamage());

	/* called whenever this character dies in the game world */
	UFUNCTION(BlueprintImplementableEvent, Category = Events)
	void OnCharacterDied(float KillingDamage, APawn* Killer, AActor* DamageCauser, FRealmDamage realmDamage);

	/* called when this character starts combat */
	void CharacterCombatAction();

	/* called when this character has been in no combat for a period of time */
	void CharacterCombatFinished();

	/* don't replicate when this unit is not visible for a player */
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	/* damage over time tick */
	void DamageOverTimeTick(FString dotKey);

	/* array of dots currently affecting this character */
	TMap<FString, FDamageOverTime> dotEvents;

	FTimerHandle clearLastHitTimer;
	AGameCharacter* lastDamagingCharacter;

	/* clear the last take hit */
	void ClearLastTakeHit();

	/* array of chraracters that we have recently damage that we can have brief sight of */
	TMap<FName, AGameCharacter*> damagedSightCharacters;
	float damagedSightTimeout;
	void RemoveDamagedSightCharacter(FName charName);

public:

	/* timers for auto attacks */
	FTimerHandle aaRangeTimer, aaTimer, aaLaunchTimer;

	/* timer for actions */
	UPROPERTY(BlueprintReadOnly, Category=Actions)
	FTimerHandle actionTimer;

	UPROPERTY(BlueprintReadOnly, Category=Respawn)
	FTimerHandle respawnTimer;

	FTimerHandle healthRegen, flareRegen;

	/* how much damage should be mitigated from the next TakeDamage call */
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	float nextMitigatedDamage;

	/* the stealth area this unit is currently occupying, if any */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = StealthArea)
	AStealthArea* currentStealthArea;

	/* radius this character can see */
	UPROPERTY(EditDefaultsOnly, Category = Sight)
	float sightRadius;

	/* check whether or not this character has movement enabled */
	bool CanMove() const;

	/* check whether or not tstophis character is able to perform skills */
	bool CanPerformSkills() const;

	/* check whether or not this character can auto attack */
	bool CanAutoAttack() const;

	/* perform the server version of the skill then perform the skill on all clients */
	UFUNCTION(NetMulticast, reliable, WithValidation)
	void UseSkill(int32 index, FVector mouseHitLoc, AGameCharacter* unitTarget);

	/* use try to use the mod on all clients */
	UFUNCTION(NetMulticast, reliable, WithValidation)
	void UseMod(int32 index, FHitResult const& hit);

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class AGameCharacter* PawnInstigator, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc);

	/* [SERVER] launch the charcater's auto attack once in range */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void StartAutoAttack();

	/* actually launch the attack (this is the point where the attack cannot be cancelled) */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void LaunchAutoAttack();

	/* try to cancel any auto attacks in progress of being performed and clear current target */
	UFUNCTION(BlueprintCallable, Category = AA)
	virtual void StopAutoAttack(bool bClearCurrrentTarget = true);

	/* this is called every 1/20th of a second while the auto attack is launched to make sure were still in range */
	virtual void CheckAutoAttack();

	/* called to reset the cooldown timer for auto attacks (for skills) */
	UFUNCTION(BlueprintCallable, Category = AA)
	void ResetAutoAttack();

	/* stop auto attack cooldown */
	virtual void OnFinishAATimer();

	/* get the current target for this character */
	UFUNCTION(BlueprintCallable, Category = Target)
	AGameCharacter* GetCurrentTarget() const;

	/* set the current target */
	UFUNCTION(BlueprintCallable, Category = Target)
	void SetCurrentTarget(AGameCharacter* newTarget);

	/* get the stat manager */
	UFUNCTION(BlueprintCallable, Category = Stats)
	UStatsManager* GetStatsManager() const;

	/* get the auto attack manager */
	UFUNCTION(BlueprintCallable, Category = AA)
	UAutoAttackManager* GetAutoAttackManager() const;

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
	UFUNCTION(BlueprintCallable, Category = Stat)
	AEffect* AddEffect(const FText& effectName, const FText& effectDescription, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration = 0.f, FString const& keyName = "", bool bStacking = false, bool bMultipleInfliction = false, bool bPersistThroughDeath = false);

	UFUNCTION(BlueprintCallable, Category = Stat)
	void AddEffectStacks( const FString& effectKey,  int32 stackAmount);

	UFUNCTION(BlueprintCallable, Category = Stat)
	void EndEffect(const FString& effectKey);

	/* damages this character over time */
	UFUNCTION(BlueprintCallable, Category = Damage)
	virtual void CharacterTakeDamageOverTime(float Damage, float damageTime, int32 tickCount, UPARAM(ref) FString& dotKey, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, UPARAM(ref) FRealmDamage& realmDamage, UPARAM(ref) FDamageRecap& damageDesc);

	/* call other things and track extra damage data */
	UFUNCTION(BlueprintCallable, Category = Damage)
	virtual float CharacterTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, UPARAM(ref) FRealmDamage& realmDamage, UPARAM(ref) FDamageRecap& damageDesc);

	/** Take damage, handle death */
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

	/** cleanup inventory */
	virtual void Destroy(bool bNetForce = false, bool bShouldModifyLevel = true);

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* Killer, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc);

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
	void CharacterDash(FVector dashVelocity, float spdScale = 1.f);

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
	void DamagedOtherCharacter(AGameCharacter* hurtCharacter, struct FDamageEvent const& DamageEvent, float damageAmount = 0.f, FRealmDamage const& realmDamage = FRealmDamage());

	/* blueprint hook to do game logic whenever this character is about to launch an auto attack */
	UFUNCTION(BlueprintImplementableEvent, Category = AutoAttack)
	void OnLaunchAutoAttack();

	/* get the amount of experience needed for the next level */
	UFUNCTION(BlueprintCallable, Category = Exp)
	int32 GetNextLevelExperience() const;

	/* give this character experience */
	UFUNCTION(BlueprintCallable, Category = Exp)
	virtual void GiveCharacterExperience(int32 amount);

	/* blueprint hook for when effects change or update (for HUDs) */
	void EffectsUpdated();

	/* called whenever something tries to give this character an Ailment */
	UFUNCTION(BlueprintCallable, Category = CC)
	void GiveCharacterAilment(FAilmentInfo info);

	/* called to get the current Ailment status of this character */
	UFUNCTION(BlueprintCallable, Category = CC)
	FAilmentInfo GetCharacterAilment() const; 

	/* called whenever an ailment currently affecting this character is finished */
	UFUNCTION(BlueprintCallable, Category = CC)
	void CurrentAilmentFinished();

	/* blueprint hook for whenever this character enters combat */
	UFUNCTION(BlueprintImplementableEvent, Category = Combat)
	void CharacterEnteredCombat();

	/* blueprint hook for whenever this character leaves combat */
	UFUNCTION(BlueprintImplementableEvent, Category = Combat)
	void CharacterLeftCombat();

	/* called whenever this character spawns (or respawns) in the game world */
	UFUNCTION(BlueprintImplementableEvent, Category = Events)
	void OnCharacterSpawned();

	/* static function for creating ailments */
	UFUNCTION(BlueprintCallable, Category = CC)
	static FAilmentInfo MakeAilmentInfo(EAilment ailment, FString ailmentString, float ailmentDuration, FVector ailmentDir);

	/* called by the fog of war manager to get visibility data */
	virtual void CalculateVisibility(TArray<AGameCharacter*>& sightList, TArray<AGameCharacter*>& availableUnits);

	/* whether or not the enemy team can see this character even if its not in their sight range */
	UFUNCTION(BlueprintCallable, Category = Vision)
	bool CanEnemyAbsolutelySeeThisUnit() const;

	/* set whether or not the enemy team can see this character even if its not in their sight range */
	UFUNCTION(BlueprintCallable, Category = Vision)
	void SetEnemyAbsolutelySeeThisUnit(bool bNewCanSee = false);

	/* set whether or not the next auto attack will critically hit */
	UFUNCTION(BlueprintCallable, Category = Vision)
	void SetGuaranteedCrit(bool bNewCrit = false);

	/* [LOCAL] plays the current auto attack's animation on the local character */
	UFUNCTION(BlueprintCallable, Category = AutoAttack)
	void PlayAutoAttackAnimation(float InPlayRate);

	/* play animation */
	UFUNCTION(NetMulticast, reliable, WithValidation)
	void AllPlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f);

	/* stop animation */
	UFUNCTION(NetMulticast, reliable, WithValidation)
	void AllStopAnimMontage(class UAnimMontage* AnimMontage);

	/* apply an action to this character */
	UFUNCTION(BlueprintCallable, NetMulticast, reliable, Category = Action)
	void ApplyCharacterAction(const FString& actionName, float actionDuration, bool bReverseProgressBar = false, bool bPreventCombat = false, bool bPreventMovement = false);

	/* initiate the character's stats to the specified level */
	void InitCharacterStatsForLevel(int32 level);

	/* get the character level */
	int32 GetLevel() const
	{
		return level;
	}

	/* get the array of mods this character has */
	UFUNCTION(BlueprintCallable, Category = Mods)
	TArray<AMod*>& GetMods()
	{
		return mods;
	}

	/* called in blueprints whenever this character needs to negate the next damage event */
	UFUNCTION(BlueprintCallable, Category = Damage)
	void SetNegateNextDamage()
	{
		bNegateNextDmgEvent = true;
	}

	/* gets the character's name */
	UFUNCTION(BlueprintCallable, Category=CharacterName)
	void GetCharacterName(FText& outText) const
	{
		outText = characterName;
	}

	/* gets the name of a specified character class */
	UFUNCTION(BlueprintCallable, Category = characterName)
	static void GetCharacterClassName(TSubclassOf<AGameCharacter> characterClass, FText& outText)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(characterClass->GetDefaultObject());
		if (IsValid(gc))
			gc->GetCharacterName(outText);
	}

	/* gets whether or not this character is targetable */
	UFUNCTION(BlueprintCallable, Category = Targetable)
	bool IsTargetable() const
	{
		return bIsTargetable;
	}

	/* sets this character's targetability */
	UFUNCTION(BlueprintCallable, Category=Targetable)
	void SetTargetable(bool bNewIsTargetable)
	{
		bIsTargetable = bNewIsTargetable;
	}

	/* server function for upgrading skills*/
	void OnUpgradeSkill(int32 index);

	/* function to call for this character to play a sound over the network ONCE */
	UFUNCTION(BlueprintCallable, reliable, NetMulticast, Category = CharacterSound)
	void PlayCharacterSound(USoundBase* sound, bool bAttachedToCharacter = false);

	/* requests to add a shield to the shield manager */
	UFUNCTION(BlueprintCallable, Category = Shield)
	void AddShield(FCharacterShield newShield);

	/* adds a specific damager to this character's list */
	UFUNCTION(BlueprintCallable, Category = Damage)
	void AddSpecificDamager(AGameCharacter* specificCharacter);

	/* sets whether or not we obey specific damagers */
	UFUNCTION(BlueprintCallable, Category = Damage)
	void SetOnlySpecificDamage(bool bNewSpecificDamage);

	/* clears the specific damager array */
	UFUNCTION(BlueprintCallable, Category = Damage)
	void ClearSpecificDamagers();

	/* function for generating a critical hit for this character which returns whethere or not the hit was critical and how much total damage the hit will do */
	UFUNCTION(BlueprintCallable, Category = CritChance)
	bool CalculateCriticalHit(float& totalDamage, float additionalCritChance = 0.f);

	/* to successfully replicate our manager subojects without having to take a performance hit */
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	/* static function to easily acquire the point on the ground directly beneath a game character */
	UFUNCTION(BlueprintCallable, Category = Ground)
	static FVector FindGroundBeneathCharacter(AGameCharacter* testCharacter);

	/* whether or not this character has vision on the specified test character */
	UFUNCTION(BlueprintCallable, Category = Vision)
	bool CanSeeOtherCharacter(AGameCharacter* testCharacter, bool bTestForThisCharacter = true);

	/* set the mesh's animation rate (useful for things like pausing the character's animation) */
	UFUNCTION(BlueprintCallable, Category = Animation)
	void SetGloabalAnimRate(float newAnimRate);

	/* blueprint hook to call whenever this character has killed a unit */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void KilledOtherCharacter(float KillingDamage, AGameCharacter* victim, AActor* DamageCauser, FRealmDamage realmDamage);

	/* whether or not this character is inflicted with the specified dot */
	UFUNCTION(BlueprintCallable, Category = Damage)
	bool HasSpecifiedDoT(FString dotKey);
};