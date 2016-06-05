#include "Realm.h"
#include "GameCharacter.h"
#include "UnrealNetwork.h"
#include "RealmMoveController.h"
#include "PlayerHUD.h"
#include "PlayerCharacter.h"
#include "Projectile.h"
#include "RealmPlayerController.h"
#include "RealmGameMode.h"
#include "RealmCharacterMovementComponent.h"
#include "RealmLaneMinionAI.h"
#include "RealmFogofWarManager.h"
#include "OverheadWidget.h"
#include "Engine/ActorChannel.h"

AGameCharacter::AGameCharacter(const FObjectInitializer& objectInitializer)
:Super(objectInitializer.SetDefaultSubobjectClass<URealmCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	//SoundAttenuation'/Game/Realm/Sounds/GeneralRealmAttenuation.GeneralRealmAttenuation'
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> soundAttObj(TEXT("/Game/Realm/Sounds/GeneralRealmAttenuation"));
	if (soundAttObj.Succeeded())
		soundAttenuation = soundAttObj.Object;

	AIControllerClass = AAIController::StaticClass();
	bAlwaysRelevant = true;

	level = 1;
	experienceAmount = 0;
	skillPoints = 0;
	baseExpReward = 21;
	experienceRewardRange = 1500.f;
	sightRadius = 710.f;
	combatTimeoutDelay = 3.5f;
	overheadHalfHeightMultiplier = 2.5f;

	nextMitigatedDamage = 0.f;

	NetUpdateFrequency = 30.f;

	lastTakeHitTimeTimeout = 2.f;
	damagedSightTimeout = 2.f;
}

void AGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		statsManager = NewObject<UStatsManager>(this);
		autoAttackManager = NewObject<UAutoAttackManager>(this);
		modManager = NewObject<UModManager>(this);

		//initialize stats
		statsManager->InitializeStats(characterData->GetDefaultObject<UGameCharacterData>()->GetCharacterBaseStats(), this);

		if (IsValid(shieldManager))
		{
			shieldManager->owningCharacter = this;
			shieldManager->SetOwner(this);
		}

		autoAttackManager->InitializeManager(autoAttacks, statsManager);
		//autoAttackManager->SetOwner(playerController);
		//statsManager->AttachRootComponentToActor(this);
 		//autoAttackManager->AttachRootComponentToActor(this);
		//modManager->SetOwner(this);

		statsManager->SetMaxHealth();
		statsManager->SetMaxFlare();

		modManager->managedCharacter = this;
	}

	for (TActorIterator<AHUD> objItr(GetWorld()); objItr; ++objItr)
	{
		APlayerHUD* hud = Cast<APlayerHUD>((*objItr));
		if (hud)
			hud->AddPostRenderedActor(this);
	}

	if (overheadWidgetClass && GetNetMode() != NM_DedicatedServer)
	{
		overheadWidget = CreateWidget<UOverheadWidget>(GetWorld(), overheadWidgetClass);
		if (overheadWidget)
		{
			overheadWidget->SetParentCharacter(this);
			overheadWidget->AddOverheadWidgetToViewport();
		}
	}
}

void AGameCharacter::Destroy(bool bNetForce /* = false */, bool bShouldModifyLevel /* = true */)
{
	//autoAttackManager->Destroy();
	//statsManager->Destroy();
	//modManager->Destroy();

	if (IsValid(skillManager))
		skillManager->Destroy();

	if (IsValid(shieldManager))
		shieldManager->Destroy();

	autoAttackManager = nullptr;
	statsManager = nullptr;
	skillManager = nullptr;
	modManager = nullptr;
	overheadWidget = nullptr;
	shieldManager = nullptr;

	Super::Destroy(bNetForce, bShouldModifyLevel);
}

void AGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Role == ROLE_Authority)
	{
		GetCharacterMovement()->MaxWalkSpeed = GetCurrentValueForStat(EStat::ES_Move);

		if (IsValid(GetStatsManager()) && IsValid(GetAutoAttackManager()))
			GetStatsManager()->baseStats[(uint8)EStat::ES_AARange] = GetAutoAttackManager()->GetCurrentAutoAttackRange();

		if (IsAlive())
		{
			if (GetHealth() < GetCurrentValueForStat(EStat::ES_HP))
			{
				if (GetWorldTimerManager().GetTimerRemaining(healthRegen) <= 0.f)
					GetWorldTimerManager().SetTimer(healthRegen, this, &AGameCharacter::HealthRegen, 0.25f, true);
			}
			else if (GetHealth() > GetCurrentValueForStat(EStat::ES_HP))
			{
				GetWorldTimerManager().ClearTimer(healthRegen);
				statsManager->health = GetCurrentValueForStat(EStat::ES_HP);
			}

			if (GetFlare() < GetCurrentValueForStat(EStat::ES_Flare))
			{
				if (GetWorldTimerManager().GetTimerRemaining(flareRegen) <= 0.f)
					GetWorldTimerManager().SetTimer(flareRegen, this, &AGameCharacter::FlareRegen, 0.25f, true);
			}
			else if (GetFlare() > GetCurrentValueForStat(EStat::ES_Flare))
			{
				GetWorldTimerManager().ClearTimer(flareRegen);
				statsManager->flare = GetCurrentValueForStat(EStat::ES_Flare);
			}
		}
	}

	if (IsValid(GetCurrentTarget()) && GetCurrentTarget()->IsAlive() && IsAlive())
	{
		FRotator newRot = GetActorRotation();
		FRotator dir = (GetCurrentTarget()->GetActorLocation() - GetActorLocation()).Rotation();
		newRot.Yaw = dir.Yaw;

		SetActorRotation(newRot);
	}

	if (!IsValid(GetWorld()->GetFirstPlayerController()))
		return;
}

void AGameCharacter::ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled, FRealmDamage& realmDamage)
{
	const float timeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	/*FDamageEvent lastDamageEvent = lastTakeHitInfo.GetDamageEvent();
	if ((instigatingPawn == lastTakeHitInfo.PawnInstigator) && (lastDamageEvent.DamageTypeClass == lastTakeHitInfo.DamageTypeClass) && (lastTakeHitTimeTimeout == timeoutTime))
	{
		// same frame damage
		if (bKilled && lastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		damage += lastTakeHitInfo.ActualDamage;
	}*/

	lastTakeHitInfo.ActualDamage = damage;
	lastTakeHitInfo.realmDamage = realmDamage;
	lastTakeHitInfo.PawnInstigator = Cast<AGameCharacter>(instigatingPawn);
	lastTakeHitInfo.DamageCauser = damageCauser;
	lastTakeHitInfo.SetDamageEvent(damageEvent);
	lastTakeHitInfo.bKilled = bKilled;
	lastTakeHitInfo.EnsureReplication();

	lastTakeHitTimeTimeout = timeoutTime;

	lastDamagingCharacter = Cast<AGameCharacter>(instigatingPawn);
	GetWorldTimerManager().SetTimer(clearLastHitTimer, this, &AGameCharacter::ClearLastTakeHit, 2.f, false);

	FTimerHandle h;
	GetWorldTimerManager().SetTimer(h, FTimerDelegate::CreateUObject(this, &AGameCharacter::RemoveDamagedSightCharacter, instigatingPawn->GetFName()), damagedSightTimeout, false);
	damagedSightCharacters.Add(instigatingPawn->GetFName(), Cast<AGameCharacter>(instigatingPawn));
}

void AGameCharacter::RemoveDamagedSightCharacter(FName charName)
{
	if (damagedSightCharacters.Contains(charName))
		damagedSightCharacters.Remove(charName);
}

void AGameCharacter::ClearLastTakeHit()
{
	lastDamagingCharacter = nullptr;
}

void AGameCharacter::OnRep_LastTakeHitInfo()
{
	if (lastTakeHitInfo.bKilled)
	{
		OnDeath(lastTakeHitInfo.ActualDamage, lastTakeHitInfo.GetDamageEvent(), lastTakeHitInfo.PawnInstigator, lastTakeHitInfo.DamageCauser.Get(), lastTakeHitInfo.realmDamage);
	}
	else if (Role < ROLE_Authority)
	{
		PlayHit(lastTakeHitInfo.ActualDamage, lastTakeHitInfo.GetDamageEvent(), lastTakeHitInfo.PawnInstigator, lastTakeHitInfo.DamageCauser.Get(), lastTakeHitInfo.realmDamage);
	}
}

bool AGameCharacter::CanMove() const
{
	return (currentAilment.newAilment != EAilment::AL_Knockup && currentAilment.newAilment != EAilment::AL_Stun && bAcceptingMoveCommands && GetCharacterMovement()->GetGroundMovementMode() == MOVE_Walking);
}

bool AGameCharacter::CanAutoAttack() const
{
	return currentAilment.newAilment != EAilment::AL_Stun && GetWorldTimerManager().GetTimerRemaining(actionTimer) <= 0.f && !bActionPreventingCombat;
}

bool AGameCharacter::CanPerformSkills() const
{
	return currentAilment.newAilment != EAilment::AL_Stun && GetWorldTimerManager().GetTimerRemaining(actionTimer) <= 0.f && !bActionPreventingCombat;
}

bool AGameCharacter::UseSkill_Validate(int32 index, FVector mouseHitLoc, AGameCharacter* unitTarget)
{
	return true;
}

void AGameCharacter::UseSkill_Implementation(int32 index, FVector mouseHitLoc, AGameCharacter* unitTarget)
{
	if (skillManager->GetSkill(index) == nullptr)
		return;

	if (!IsAlive())
		return;

	if (Role < ROLE_Authority || (GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone))
	{
		skillManager->ClientPerformSkill(index, mouseHitLoc, unitTarget);
		return;
	}

	ESkillState skillState = skillManager->GetSkill(index)->GetSkillState();
	if (skillState == ESkillState::Disabled || skillState == ESkillState::OnCooldown || skillState == ESkillState::Performing || skillState == ESkillState::NotLearned)
		return;

	float curr = statsManager->GetFlare();
	float diff = curr - skillManager->GetSkill(index)->GetCost();
	if (diff < 0)
		return;

	if (skillManager->GetSkill(index)->bAutoPerform)
		skillManager->GetSkill(index)->SetSkillState(ESkillState::Performing);

	if (Role == ROLE_Authority)
	{
		skillManager->ServerPerformSkill(index, mouseHitLoc, unitTarget);
		CharacterCombatAction();
	}
}

bool AGameCharacter::UseMod_Validate(int32 index, FHitResult const& hit)
{
	return true;
}

void AGameCharacter::UseMod_Implementation(int32 index, FHitResult const& hit)
{
	if (index >= mods.Num())
		return;

	if (!IsAlive() || !IsValid(mods[index]))
		return;

	if (mods[index]->GetCooldownRemaining() > 0.f)
		return;

	if (currentAilment.newAilment == EAilment::AL_Stun || GetWorldTimerManager().GetTimerRemaining(actionTimer) > 0.f)
		return;

	if (Role == ROLE_Authority)
		mods[index]->ServerModUsed(hit);
	else
		mods[index]->ClientModUsed(hit);
}

void AGameCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class AGameCharacter* PawnInstigator, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	if (!IsValid(this))
		return;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false, realmDamage);
	}
	
	if ((IsValid(PawnInstigator) && PawnInstigator->playerController == GetWorld()->GetFirstPlayerController()) || playerController == GetWorld()->GetFirstPlayerController())
	{
		if (IsValid(PawnInstigator->playerController))
		{
			AHUD* hud = PawnInstigator->playerController->GetHUD();
			APlayerHUD* InstigatorHUD = Cast<APlayerHUD>(hud);
			if (IsValid(InstigatorHUD))
				InstigatorHUD->NewDamageEvent(lastTakeHitInfo, GetActorLocation(), realmDamage);
		}
		else if (IsValid(playerController))
		{
			AHUD* hud = playerController->GetHUD();
			APlayerHUD* InstigatorHUD = Cast<APlayerHUD>(hud);
			if (IsValid(InstigatorHUD))
				InstigatorHUD->NewDamageEvent(lastTakeHitInfo, GetActorLocation(), realmDamage);
		}
	}

	/*AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	AShooterHUD* MyHUD = MyPC ? Cast<AShooterHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyWeaponHit(DamageTaken, DamageEvent, PawnInstigator);
	}*/
}

void AGameCharacter::StartAutoAttack()
{
	if (!IsValid(GetCurrentTarget()) || !IsValid(autoAttackManager) || !IsValid(statsManager) || !IsAlive())
		return;

	if (bAutoAttackLaunching) //bAutoAttackOnCooldown || bAutoAttackLaunching
		return;

	if (GetWorldTimerManager().GetTimerRemaining(aaRangeTimer) <= 0.f)
		GetWorldTimerManager().SetTimer(aaRangeTimer, this, &AGameCharacter::CheckAutoAttack, 1.f / 20.f, true);

	if (bAutoAttackOnCooldown)
	{
		GetWorldTimerManager().ClearTimer(aaRangeTimer);
		return;
	}

	float distance = (GetActorLocation() - GetCurrentTarget()->GetActorLocation()).Size2D();
	if (distance <= statsManager->GetCurrentValueForStat(EStat::ES_AARange))
	{
		ARealmMoveController* aicc = Cast<ARealmMoveController>(GetController());
		if (IsValid(aicc))
			aicc->CharacterInAttackRange();

		AAIController* aic = Cast<AAIController>(GetController());
		if (IsValid(aic))
			aic->StopMovement();

		float scale = statsManager->GetCurrentValueForStat(EStat::ES_AtkSp) / statsManager->GetBaseValueForStat(EStat::ES_AtkSp);
		GetWorldTimerManager().SetTimer(aaLaunchTimer, this, &AGameCharacter::LaunchAutoAttack, autoAttackManager->GetAutoAttackLaunchTime() / scale);

		bAutoAttackLaunching = true;
	}
}

void AGameCharacter::LaunchAutoAttack()
{
	//GetWorldTimerManager().ClearTimer(aaRangeTimer);

	if (!IsValid(GetCurrentTarget()) || !IsValid(this) || !IsValid(GetController()) || !GetCurrentTarget()->IsAlive())
	{
		SetCurrentTarget(nullptr);
		bAutoAttackLaunching = false;

		ARealmLaneMinionAI* aic = Cast<ARealmLaneMinionAI>(GetController());
		if (IsValid(aic))
			aic->NeedsNewCommand();

		return;
	}

	FRealmDamage rdmg;

	//set the controlling character as the damager if there is one
	rdmg.controllingCharacter = controllingCharacter;

	//calculate critcal hit
	float dmg = GetCurrentValueForStat(EStat::ES_Atk);

	if (currentAilment.newAilment == EAilment::AL_Blind)
		dmg = 0.f;

	if (bGuaranteeCrit)
	{
		dmg += dmg * (GetCurrentValueForStat(EStat::ES_CritRatio) / 100.f);
		bGuaranteeCrit = false;
		rdmg.bCriticalHit = true;
	}
	else if (CalculateCriticalHit(dmg))
		rdmg.bCriticalHit = true;

	ATurret* tr = Cast<ATurret>(this);
	if (IsValid(tr))
		rdmg.damageSource = ERealmDamageSource::ERDS_Turret;

	//play launch sound
	if (autoAttackManager->GetCurrentAutoAttackLaunchSound())
		PlayCharacterSound(autoAttackManager->GetCurrentAutoAttackLaunchSound());

	if (autoAttackManager->IsCurrentAttackProjectile())
	{
		//launch a projectile
		FVector spawnPos = GetMesh()->GetSocketLocation(autoAttackManager->GetCurrentAutoAttackProjectileSocket());
		FRotator dir = (GetCurrentTarget()->GetActorLocation() - GetActorLocation()).Rotation();
		AProjectile* attackProjectile = GetWorld()->SpawnActor<AProjectile>(autoAttackManager->GetCurrentAutoAttackProjectileClass(), spawnPos, dir);

		if (IsValid(attackProjectile))
		{
			attackProjectile->bAutoAttackProjectile = true;
			attackProjectile->hitSound = autoAttackManager->GetCurrentAutoAttackHitSound();
			attackProjectile->InitializeProjectile(dir.Vector(), dmg, UPhysicalDamage::StaticClass(), this, GetCurrentTarget(), rdmg);
		}
	}
	else
	{
		//sound
		if (autoAttackManager->GetCurrentAutoAttackHitSound())
			PlayCharacterSound(autoAttackManager->GetCurrentAutoAttackHitSound());

		//instant damage
		FDamageEvent de(UPhysicalDamage::StaticClass());

		GetCurrentTarget()->CharacterTakeDamage(dmg, de, GetRealmController(), this, rdmg);
		//GetCurrentTarget()->TakeDamage(dmg, de, GetRealmController(), this);
	}

	bAutoAttackLaunching = false;

	float aaTime = 1.f / statsManager->GetCurrentValueForStat(EStat::ES_AtkSp);
	GetWorldTimerManager().SetTimer(aaTimer, this, &AGameCharacter::OnFinishAATimer, aaTime);
	bAutoAttackOnCooldown = true;

	GetWorldTimerManager().ClearTimer(aaRangeTimer);
}

bool AGameCharacter::CalculateCriticalHit(float& totalDamage, float additionalCritChance)
{
	float critPercent = GetCurrentValueForStat(EStat::ES_CritChance) + additionalCritChance;
	if (critPercent <= 0.f)
		return false;

	float crit = FMath::RandRange(0, 100);
	if (crit <= critPercent)
	{
		totalDamage += totalDamage * (GetCurrentValueForStat(EStat::ES_CritRatio) / 100.f);
		return true;
	}
	else
		return false;
}

void AGameCharacter::CheckAutoAttack()
{
	if (!IsValid(GetCurrentTarget()) || !IsValid(GetController()) || !GetCurrentTarget()->IsAlive() || GetCurrentTarget()->bHidden)
	{
		SetCurrentTarget(nullptr);
		GetWorldTimerManager().ClearTimer(aaLaunchTimer);
		bAutoAttackLaunching = false;

		return;
	}

	//if the attack is already >= 75% launched, launch anyway so there's less stuttering when auto attacking
	if (bAutoAttackLaunching)
	{
		if (GetWorldTimerManager().GetTimerElapsed(aaLaunchTimer) / (GetWorldTimerManager().GetTimerElapsed(aaLaunchTimer) + GetWorldTimerManager().GetTimerRemaining(aaLaunchTimer)) >= 0.75f)
			return;
	}

	//make sure that we account for collision so we don't get stuck trying to attack
	FVector start = GetActorLocation();
	FVector end = GetCurrentTarget()->GetActorLocation();
	TArray<FHitResult> hits;

	GetWorld()->LineTraceMultiByChannel(hits, start, end, ECC_Pawn);
	for (FHitResult hit : hits)
	{
		if (hit.GetActor() == GetCurrentTarget())
			end = hit.ImpactPoint;
	}

	float distance = (end - start).Size2D();
	if (distance > statsManager->GetCurrentValueForStat(EStat::ES_AARange) && !bAutoAttackLaunching)
	{
		GetWorldTimerManager().ClearTimer(aaLaunchTimer);
		bAutoAttackLaunching = false;

		if (CanSeeOtherCharacter(currentTarget))
		{
			AAIController* aic = Cast<AAIController>(GetController());
			if (IsValid(aic))
				aic->MoveToActor(GetCurrentTarget(), statsManager->GetCurrentValueForStat(EStat::ES_AARange));
		}
	}
	else if (!bAutoAttackLaunching && !bAutoAttackOnCooldown)
	{
		AAIController* aic = Cast<AAIController>(GetController());
		if (IsValid(aic))
			aic->StopMovement();

		ARealmMoveController* aicc = Cast<ARealmMoveController>(GetController());
		if (IsValid(aicc))
			aicc->CharacterInAttackRange();

		float scale = statsManager->GetCurrentValueForStat(EStat::ES_AtkSp) / statsManager->GetBaseValueForStat(EStat::ES_AtkSp);
		GetWorldTimerManager().SetTimer(aaLaunchTimer, this, &AGameCharacter::LaunchAutoAttack, autoAttackManager->GetAutoAttackLaunchTime() / scale);

		bAutoAttackLaunching = true;
	}
}

void AGameCharacter::ResetAutoAttack()
{
	OnFinishAATimer();
}

void AGameCharacter::OnFinishAATimer()
{
	bAutoAttackOnCooldown = false;
	bAutoAttackLaunching = false;

	if (IsValid(GetCurrentTarget()) && GetCurrentTarget()->IsAlive() && !GetCurrentTarget()->bHidden)
		StartAutoAttack();
	else
		StopAutoAttack();
}

void AGameCharacter::StopAutoAttack()
{
	SetCurrentTarget(nullptr);

	if (bAutoAttackLaunching && autoAttackManager)
		AllStopAnimMontage(autoAttackManager->GetCurrentAttackAnimation());

	GetWorldTimerManager().ClearTimer(aaLaunchTimer);
	GetWorldTimerManager().ClearTimer(aaRangeTimer);
	bAutoAttackLaunching = false;
}

void AGameCharacter::PlayAutoAttackAnimation(float InPlayRate)
{
	if (!autoAttackManager)
		return;

	USkeletalMeshComponent* UseMesh = GetMesh();
	if (autoAttackManager->GetCurrentAttackAnimation() && UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Play(autoAttackManager->GetCurrentAttackAnimation(), InPlayRate);
	}
}

bool AGameCharacter::AllPlayAnimMontage_Validate(class UAnimMontage* AnimMontage, float InPlayRate)
{
	return true;
}

bool AGameCharacter::AllStopAnimMontage_Validate(class UAnimMontage* AnimMontage)
{
	return true;
}

void AGameCharacter::AllPlayAnimMontage_Implementation(class UAnimMontage* AnimMontage, float InPlayRate)
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}
}

void AGameCharacter::AllStopAnimMontage_Implementation(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);
	}
}

AGameCharacter* AGameCharacter::GetCurrentTarget() const
{
	return (!IsAlive() || !IsValid(this)) ? nullptr : currentTarget;
}

void AGameCharacter::SetCurrentTarget(AGameCharacter* newTarget)
{
	if (IsAlive() && IsValid(this) && IsValid(newTarget))
		currentTarget = newTarget;
	else
		currentTarget = nullptr;
}

UStatsManager* AGameCharacter::GetStatsManager() const
{
	return statsManager;
}

UAutoAttackManager* AGameCharacter::GetAutoAttackManager() const
{
	return autoAttackManager;
}

ASkillManager* AGameCharacter::GetSkillManager() const
{
	return skillManager;
}

float AGameCharacter::GetCurrentValueForStat(EStat stat) const
{
	if (statsManager)
		return statsManager->GetCurrentValueForStat(stat);
	else
		return -1.f;
}

float AGameCharacter::GetBaseValueForStat(EStat stat) const
{
	if (statsManager)
		return statsManager->GetBaseValueForStat(stat);
	else
		return -1.f;
}

float AGameCharacter::GetUnaffectedValueForStat(EStat stat) const
{
	if (statsManager)
		return statsManager->GetUnaffectedValueForStat(stat);
	else
		return -1.f;
}

AEffect* AGameCharacter::AddEffect(const FText& effectName, const FText& effectDescription, const TArray<TEnumAsByte<EStat> >& stats, const TArray<float>& amounts, float effectDuration, FString const& keyName, bool bStacking, bool bMultipleInfliction)
{
	if (Role == ROLE_Authority && statsManager)
		return statsManager->AddEffect(effectName, effectDescription, stats, amounts, effectDuration, keyName, bStacking, bMultipleInfliction);
	else
		return nullptr;
}

void AGameCharacter::AddEffectStacks(const FString& effectKey, int32 stackAmount)
{
	if (Role == ROLE_Authority && IsValid(statsManager))
		statsManager->AddEffectStacks(effectKey, stackAmount);
}

void AGameCharacter::EndEffect(const FString& effectKey)
{
	if (Role == ROLE_Authority && IsValid(statsManager))
		statsManager->EffectFinished(effectKey);
}


void AGameCharacter::CharacterTakeDamageOverTime(float Damage, float damageTime, int32 tickCount, FString& dotKey, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, UPARAM(ref) FRealmDamage& realmDamage)
{
	if (Role < ROLE_Authority)
		return;

	if (!IsValid(statsManager) || !IsAlive())
		return;

	if (dotEvents.Contains(dotKey))
	{
		//reset dots if they already exist
		dotEvents[dotKey].incurredTickDamage = 0.f;
		return;
	}

	if (damageTime > 0.f && Damage > 0.f)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>(EventInstigator);
		ARealmMoveController* aipc = Cast<ARealmMoveController>(EventInstigator);
		AGameCharacter* damageCausingGC = NULL;

		//subtract the right type of defense for the tot DoT
		if (!GetWorld()->GetAuthGameMode<ARealmGameMode>()->CanDamageFriendlies() && ((pc && pc->GetPlayerCharacter()->GetTeamIndex() == teamIndex) || (damageCausingGC && damageCausingGC->GetTeamIndex() == teamIndex)))
			return;

		if (DamageEvent.DamageTypeClass == UPhysicalDamage::StaticClass() && statsManager->GetCurrentValueForStat(EStat::ES_Def) >= 0)
			Damage -= statsManager->GetCurrentValueForStat(EStat::ES_Def);
		else if (DamageEvent.DamageTypeClass == USpecialDamage::StaticClass() && statsManager->GetCurrentValueForStat(EStat::ES_SpDef) >= 0)
			Damage -= statsManager->GetCurrentValueForStat(EStat::ES_SpDef);
		
		FDamageOverTime dot;
		FTimerHandle dotTimer;

		GetWorldTimerManager().SetTimer(dotTimer, FTimerDelegate::CreateUObject(this, &AGameCharacter::DamageOverTimeTick, dotKey), damageTime / (float)tickCount, true);
		dot.DamageCauser = DamageCauser;
		dot.DamageEvent = DamageEvent;
		dot.dotTimer = dotTimer;
		dot.EventInstigator = EventInstigator;
		dot.realmDamage = realmDamage;
		dot.tickDamage = Damage / (float)tickCount;
		dot.tickDamageTotal = Damage;
		dot.tickInterval = damageTime / (float)tickCount;
		dot.dotDuration = damageTime;

		dotEvents.Add(dotKey, dot);
		DamageOverTimeTick(dotKey);
	}
}

void AGameCharacter::DamageOverTimeTick(FString dotKey)
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(dotEvents[dotKey].EventInstigator);
	ARealmMoveController* aipc = Cast<ARealmMoveController>(dotEvents[dotKey].EventInstigator);
	AGameCharacter* damageCausingGC = NULL;

	if (aipc)
	{
		damageCausingGC = Cast<AGameCharacter>(aipc->GetCharacter());
		aipc->CharacterDamaged(this);
	}
	else if (pc)
	{
		damageCausingGC = pc->GetPlayerCharacter();
		pc->GetMoveController()->CharacterDamaged(this);
	}

	if (IsValid(dotEvents[dotKey].realmDamage.controllingCharacter))
		damageCausingGC = dotEvents[dotKey].realmDamage.controllingCharacter;

	if (bOnlySpecificCharactersCanDamage && !specificDamagingCharacters.Contains(damageCausingGC))
		return;

	CharacterDamaged(dotEvents[dotKey].tickDamage, dotEvents[dotKey].DamageEvent.DamageTypeClass, damageCausingGC, dotEvents[dotKey].DamageCauser);

	if (IsValid(damageCausingGC))
	{
		damageCausingGC->HurtAnother(this, dotEvents[dotKey].DamageEvent, dotEvents[dotKey].tickDamage, dotEvents[dotKey].realmDamage);
		damageCausingGC->modManager->CharacterDealtDamage(dotEvents[dotKey].tickDamage, dotEvents[dotKey].DamageEvent.DamageTypeClass, dotEvents[dotKey].DamageCauser, dotEvents[dotKey].realmDamage, this);
	}

	CharacterCombatAction();

	if (IsValid(skillManager))
	{
		TArray<ASkill*> skills;
		skillManager->GetSkills(skills);
		for (int32 i = 0; i < skills.Num(); i++)
			skills[i]->InterruptSkill(ESkillInterruptReason::SIR_Damaged);
	}

	if (GetHealth() - dotEvents[dotKey].tickDamage > 0)
		PlayHit(dotEvents[dotKey].tickDamage, dotEvents[dotKey].DamageEvent, damageCausingGC, dotEvents[dotKey].DamageCauser, dotEvents[dotKey].realmDamage);
	else
		Die(dotEvents[dotKey].tickDamage, dotEvents[dotKey].DamageEvent, damageCausingGC, dotEvents[dotKey].DamageCauser, dotEvents[dotKey].realmDamage);

	TakeDamage(dotEvents[dotKey].tickDamage, dotEvents[dotKey].DamageEvent, dotEvents[dotKey].EventInstigator, dotEvents[dotKey].DamageCauser);

	dotEvents[dotKey].incurredTickDamage += dotEvents[dotKey].tickInterval;
	if (dotEvents[dotKey].incurredTickDamage >= dotEvents[dotKey].dotDuration)
	{
		GetWorldTimerManager().ClearTimer(dotEvents[dotKey].dotTimer);
		dotEvents.Remove(dotKey);
	}
}

float AGameCharacter::CharacterTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(EventInstigator);
	ARealmMoveController* aipc = Cast<ARealmMoveController>(EventInstigator);
	AGameCharacter* damageCausingGC = NULL;

	if (Role < ROLE_Authority)
		return 0.f;

	if (!IsValid(statsManager) || !IsAlive())
	{
		return 0.f;
	}

	if (!GetWorld()->GetAuthGameMode<ARealmGameMode>()->CanDamageFriendlies() && ((pc && pc->GetPlayerCharacter()->GetTeamIndex() == teamIndex) || (damageCausingGC && damageCausingGC->GetTeamIndex() == teamIndex)))
		return 0.f;

	if (DamageEvent.DamageTypeClass == UPhysicalDamage::StaticClass() && statsManager->GetCurrentValueForStat(EStat::ES_Def) >= 0)
		Damage -= statsManager->GetCurrentValueForStat(EStat::ES_Def);
	else if (DamageEvent.DamageTypeClass == USpecialDamage::StaticClass() && statsManager->GetCurrentValueForStat(EStat::ES_SpDef) >= 0)
		Damage -= statsManager->GetCurrentValueForStat(EStat::ES_SpDef);

	if (shieldManager)
		Damage = shieldManager->TryAbsorbDamage(Damage, DamageEvent.DamageTypeClass);

	if (aipc)
	{
		damageCausingGC = Cast<AGameCharacter>(aipc->GetCharacter());
		aipc->CharacterDamaged(this);
	}
	else if (pc)
	{
		damageCausingGC = pc->GetPlayerCharacter();
		pc->GetMoveController()->CharacterDamaged(this);
	}

	if (IsValid(realmDamage.controllingCharacter))
		damageCausingGC = realmDamage.controllingCharacter;

	if (bOnlySpecificCharactersCanDamage && !specificDamagingCharacters.Contains(damageCausingGC))
		return 0.f;

	CharacterDamaged(Damage, DamageEvent.DamageTypeClass, damageCausingGC, DamageCauser);
	
	if (IsValid(damageCausingGC))
	{
		damageCausingGC->HurtAnother(this, DamageEvent, Damage, realmDamage);
		damageCausingGC->modManager->CharacterDealtDamage(Damage, DamageEvent.DamageTypeClass, DamageCauser, realmDamage, this);
	}

	CharacterCombatAction();

	if (IsValid(skillManager))
	{
		TArray<ASkill*> skills;
		skillManager->GetSkills(skills);
		for (int32 i = 0; i < skills.Num(); i++)
			skills[i]->InterruptSkill(ESkillInterruptReason::SIR_Damaged);
	}
	
	if (bNegateNextDmgEvent)
	{
		bNegateNextDmgEvent = false;
		return 0.f;
	}

	if (Damage > 0.f)
	{
		if (GetHealth() - Damage > 0)
			PlayHit(Damage, DamageEvent, damageCausingGC, DamageCauser, realmDamage);
		else
			Die(Damage, DamageEvent, damageCausingGC, DamageCauser, realmDamage);
	}

	return TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AGameCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(EventInstigator);
	ARealmMoveController* aipc = Cast<ARealmMoveController>(EventInstigator);
	AGameCharacter* damageCausingGC = NULL;

	if (aipc)
		damageCausingGC = Cast<AGameCharacter>(aipc->GetCharacter());
	else if (pc)
		damageCausingGC = pc->GetPlayerCharacter();

	if (!damageCausingGC)
		return 0.f;

	float ActualDamage = Damage;

	if (nextMitigatedDamage >= 0.f)
	{
		ActualDamage -= nextMitigatedDamage;
		nextMitigatedDamage = 0.f;
	}
	ActualDamage = FMath::Max(ActualDamage, 0.f);

	if (ActualDamage > 0.f)
	{
		if (GetHealth() - ActualDamage > 0)
			statsManager->RemoveHealth(ActualDamage);
		else
			statsManager->RemoveHealth(GetHealth());
		
		if (IsValid(modManager))
			modManager->CharacterDamaged(ActualDamage, DamageEvent.DamageTypeClass, damageCausingGC, DamageCauser, lastTakeHitInfo.realmDamage);

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);

		if (!GetWorldTimerManager().IsTimerActive(healthRegen) && IsAlive())
			GetWorldTimerManager().SetTimer(healthRegen, this, &AGameCharacter::HealthRegen, 0.25f, true);
	}

	return ActualDamage;
}

float AGameCharacter::GetHealth() const
{
	if (IsValid(statsManager))
		return statsManager->GetHealth();
	else
		return -1.f;
}

float AGameCharacter::GetFlare() const
{
	if (IsValid(statsManager))
		return statsManager->GetFlare();
	else
		return -1.f;
}

void AGameCharacter::UseFlare(float amount)
{
	if (IsValid(statsManager))
		statsManager->RemoveFlare(amount);

	if (!GetWorldTimerManager().IsTimerActive(healthRegen))
		GetWorldTimerManager().SetTimer(flareRegen, this, &AGameCharacter::FlareRegen, 0.25f, true);
}

void AGameCharacter::HealthRegen()
{
	float hr = statsManager->GetCurrentValueForStat(EStat::ES_HPRegen) / 4.f;
	if (GetHealth() + hr < statsManager->GetCurrentValueForStat(EStat::ES_HP))
		statsManager->RemoveHealth(hr * -1);
	else
	{
		statsManager->SetMaxHealth();
		GetWorldTimerManager().ClearTimer(healthRegen);
	}
}

void AGameCharacter::FlareRegen()
{
	float hr = statsManager->GetCurrentValueForStat(EStat::ES_FlareRegen) / 4.f;
	if (GetFlare() + hr < statsManager->GetCurrentValueForStat(EStat::ES_Flare))
		statsManager->RemoveFlare(hr * -1);
	else
	{
		statsManager->SetMaxFlare();
		GetWorldTimerManager().ClearTimer(flareRegen);
	}
}

bool AGameCharacter::IsAlive() const
{
	return !bIsDying;
}

bool AGameCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, APawn* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode() == NULL
		|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}

void AGameCharacter::KilledBy(APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !bIsDying)
	{
		AController* Killer = NULL;
		AGameCharacter* gc = Cast<AGameCharacter>(EventInstigator);
		if (gc != NULL)
		{
			Killer = gc->GetPlayerController();
			LastHitBy = NULL;
		}

		FRealmDamage dmg;
		Die(GetHealth(), FDamageEvent(UDamageType::StaticClass()), NULL, NULL, dmg);
	}
}

void AGameCharacter::Suicide()
{
	KilledBy(this);
}

bool AGameCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, APawn* Killer, AActor* DamageCauser, FRealmDamage& realmDamage)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	if (!IsValid(statsManager))
		return false;

	if (Role == ROLE_Authority)
	{
		TArray<AEffect*> charEffects;
		statsManager->GetEffects(charEffects);
		for (AEffect* effect : charEffects)
		{
			if (effect->bIsTransferredToPlayerKiller)
			{
				APlayerCharacter* gc = Cast<APlayerCharacter>(Killer);
				if (IsValid(gc))
				{
					if (!IsValid(gc->AddEffect(effect->uiName, effect->description, effect->stats, effect->amounts, effect->duration, effect->keyName, effect->bStacking, effect->bCanBeInflictedMultipleTimes)))
					{
						if (!IsValid(gc->statsManager))
							break;

						AEffect* exisitingEffect = gc->statsManager->GetEffect(effect->keyName);
						if (IsValid(exisitingEffect))
							exisitingEffect->ResetEffectTimer();
					}
				}
			}
		}

		statsManager->RemoveAllEffects();
		
		if (IsValid(skillManager))
		{
			TArray<ASkill*> skills;
			skillManager->GetSkills(skills);
			for (int32 i = 0; i < skills.Num(); i++)
				skills[i]->InterruptSkill(ESkillInterruptReason::SIR_Died);
		}
	}

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();

	if (IsValid(Killer))
		Killer = GetDamageInstigator(Killer->GetController(), *DamageType)->GetPawn();

	ARealmPlayerController* const KilledPlayer = (Controller != NULL) ? Cast<ARealmPlayerController>(Controller) : Cast<ARealmPlayerController>(GetOwner());
	//GetWorld()->GetAuthGameMode<AShooterGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	StopAutoAttack();
	bAutoAttackOnCooldown = false;
	OnDeath(KillingDamage, DamageEvent, Killer, DamageCauser, realmDamage);

	return true;
}

void AGameCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	if (bIsDying)
	{
		return;
	}

	//bReplicateMovement = false;
	bIsDying = true;

	StopAnimMontage();
	StopAutoAttack();

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	AGameCharacter* gc = Cast<AGameCharacter>(PawnInstigator);
	if (!IsValid(gc))
		return;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true, realmDamage);

		TArray<APlayerCharacter*> gcs;
		for (TActorIterator<APlayerCharacter> gcitr(GetWorld()); gcitr; ++gcitr)
		{
			APlayerCharacter* gc = (*gcitr);
			if (!IsValid(gc))
				continue;

			float distsq = (gc->GetActorLocation() - GetActorLocation()).SizeSquared2D();
			if (distsq <= FMath::Square(experienceRewardRange) && GetTeamIndex() != gc->GetTeamIndex() && gc != PawnInstigator && gc->IsAlive())
				gcs.AddUnique(gc);
		}

		if (IsValid(gc))
			gc->GiveCharacterExperience((baseExpReward + (level * 2.45f)));

		for (APlayerCharacter* gcc : gcs)
		{
			if (gcc != gc)
				gcc->GiveCharacterExperience(baseExpReward / gcs.Num());
		}

		OnCharacterDied(KillingDamage, PawnInstigator, DamageCauser, realmDamage);
	}

	GetWorldTimerManager().ClearTimer(flareRegen);
	GetWorldTimerManager().ClearTimer(healthRegen);
	GetWorldTimerManager().ClearTimer(clearLastHitTimer);

	if ((IsValid(gc) && gc->playerController == GetWorld()->GetFirstPlayerController()) || playerController == GetWorld()->GetFirstPlayerController())
	{
		if (IsValid(gc->playerController))
		{
			AHUD* hud = gc->playerController->GetHUD();
			APlayerHUD* InstigatorHUD = Cast<APlayerHUD>(hud);
			if (IsValid(InstigatorHUD))
				InstigatorHUD->NewDamageEvent(lastTakeHitInfo, GetActorLocation(), realmDamage);
		}
		else if (IsValid(playerController))
		{
			AHUD* hud = playerController->GetHUD();
			APlayerHUD* InstigatorHUD = Cast<APlayerHUD>(hud);
			if (IsValid(InstigatorHUD))
				InstigatorHUD->NewDamageEvent(lastTakeHitInfo, GetActorLocation(), realmDamage);
		}
	}

	// cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	/*if (GetNetMode() != NM_DedicatedServer && deathSound && GetPawnMesh() && GetPawnMesh()->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, deathSound, GetActorLocation());
	}*/

	//DetachFromControllerPendingDestroy();

	// disable collisions on capsule

	/*if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}

	SetActorEnableCollision(true);

	// Death anim
	//float DeathAnimDuration = PlayAnimMontage(deathAnim);

	// Ragdoll
	/*if (DeathAnimDuration > 0.f)
	{
		FTimerHandle th;
		GetWorldTimerManager().SetTimer(th, this, &AFissureCharacter::SetRagdollPhysics, FMath::Min(0.1f, DeathAnimDuration), false);
	}
	else
	{
		SetRagdollPhysics();
	}

	SetRagdollPhysics();*/
}

void AGameCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	//GetCharacterMovement()->DisableMovement();
}

void AGameCharacter::StopRagdollPhysics()
{
	if (IsPendingKill())
	{
		return;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		return;
	}

	GetMesh()->PutAllRigidBodiesToSleep();
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	//GetMesh()->SetAllBodiesPhysicsBlendWeight(0.f);
	GetMesh()->bBlendPhysics = false;
	GetMesh()->AttachParent = GetRootComponent();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("BlockAll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
}

int32 AGameCharacter::GetTeamIndex() const
{
	return teamIndex;
}

void AGameCharacter::SetTeamIndex(int32 newTeam)
{
	teamIndex = newTeam;
}

void AGameCharacter::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(Cast<ARealmPlayerController>(PC)) || !IsValid(Cast<ARealmPlayerController>(PC)->GetPlayerCharacter()))
		return;

	int32 otherTeam = Cast<ARealmPlayerController>(PC)->GetPlayerCharacter()->GetTeamIndex();
	otherTeam == teamIndex ? Canvas->SetDrawColor(FColor::Blue) : Canvas->SetDrawColor(FColor::Red);

	if (!overheadWidget)
		return;

	if (IsAlive())
	{
		overheadWidget->SetVisibility(ESlateVisibility::Visible);

		if (bHidden)
			overheadWidget->SetVisibility(ESlateVisibility::Hidden);
		else
			overheadWidget->SetVisibility(ESlateVisibility::Visible);

		FVector hudPos = GetActorLocation();
		hudPos.Z += GetSimpleCollisionHalfHeight() * overheadHalfHeightMultiplier;

		FVector screenPos = Canvas->Project(hudPos);
		screenPos.X -= overheadWidget->GetDesiredSize().X / 2.f;

		overheadWidget->SetPositionInViewport(FVector2D(screenPos.X, screenPos.Y));
	}
	else
		overheadWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AGameCharacter::AddMod(AMod* newMod)
{
	if (GetModCount() + 1 > 5)
		return;

	if (IsValid(newMod))
		mods.Add(newMod);

	statsManager->UpdateModStats(mods);
}

void AGameCharacter::RemoveMod(int32 index)
{
	if (index < 0 || index >= mods.Num())
		return;

	mods.RemoveAt(index);
	statsManager->UpdateModStats(mods);
}

int32 AGameCharacter::GetModCount()
{
	return mods.Num();
}

void AGameCharacter::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{

}

AController* AGameCharacter::GetRealmController() const
{
	if (IsValid(GetPlayerController()))
		return GetPlayerController();
	else
		return GetController();
}

void AGameCharacter::CharacterDash(FVector dashEndLocation)
{
	//UE_LOG(LogCharacter, Verbose, TEXT("ACharacter::LaunchCharacter '%s' (%f,%f,%f)"), *GetName(), LaunchVelocity.X, LaunchVelocity.Y, LaunchVelocity.Z);

	URealmCharacterMovementComponent* rmc = Cast<URealmCharacterMovementComponent>(GetCharacterMovement());
	if (rmc)
	{
		//account for impassible geometry
		TArray<FHitResult> hits;
		GetWorld()->LineTraceMultiByChannel(hits, GetActorLocation(), dashEndLocation, ECC_Camera);
		for (int32 i = 0; i < hits.Num(); i++)
		{
			if ((IsValid(hits[i].GetActor()) && hits[i].GetActor()->ActorHasTag("impassible")) || (IsValid(hits[i].GetComponent()) && hits[i].GetComponent()->ComponentHasTag("impassible")))
				dashEndLocation = hits[i].ImpactPoint;
		}

		//perform dash
		rmc->DashLaunch(dashEndLocation);
		CharacterDashStarted();

		bAcceptingMoveCommands = false;
	}
}

void AGameCharacter::EndCharacterDash()
{
	URealmCharacterMovementComponent* rmc = Cast<URealmCharacterMovementComponent>(GetCharacterMovement());
	if (rmc)
		rmc->EndDash();
}

int32 AGameCharacter::GetNextLevelExperience() const
{
	return level + 1 <= MAX_LEVEL ? FMath::Square(level + 1) / FMath::Square(EXP_CONST) : 0;
}

void AGameCharacter::GiveCharacterExperience(int32 amount)
{
	if (level < MAX_LEVEL)
	{
		experienceAmount += amount;
		if (experienceAmount >= GetNextLevelExperience())
			LevelUp();
	}
	else
		experienceAmount = EXP_CONST;
}

void AGameCharacter::LevelUp()
{
	if (level + 1 > 15)
		return;

	level++;
	skillPoints++;

	if (Role == ROLE_Authority)
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerLeveledUp();

	if (IsValid(statsManager))
		statsManager->CharacterLevelUp();
}

void AGameCharacter::InitCharacterStatsForLevel(int32 newlevel)
{
	int32 deltaLvl = newlevel - level;

	if (deltaLvl > 0)
	{
		for (int32 i = 0; i < deltaLvl; i++)
			LevelUp();
	}
}

void AGameCharacter::EffectsUpdated()
{
	for (TActorIterator<APlayerHUD> huds(GetWorld()); huds; ++huds)
	{
		APlayerHUD* ph = (*huds);
		ph->CharacterEffectsUpdated(this);
	}
}

void AGameCharacter::OnRepAilment()
{
	GetWorldTimerManager().SetTimer(ailmentTimer, currentAilment.ailmentDuration, false);
}

void AGameCharacter::OnRep_AutoAttackLaunching()
{
	if (bAutoAttackLaunching)
	{
		//play the attack animation on the local client
		float scale = 1.f;
		if (statsManager)
			scale = statsManager->GetCurrentValueForStat(EStat::ES_AtkSp) / statsManager->GetBaseValueForStat(EStat::ES_AtkSp);

		PlayAutoAttackAnimation(scale);
	}
	else
	{
		/*//stop the attack animation if it's playing
		if (!autoAttackManager)
			return;

		USkeletalMeshComponent* UseMesh = GetMesh();
		UAnimMontage* AnimMontage = autoAttackManager->GetCurrentAttackAnimation();

		if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance && UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
			UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);*/
	}
}

void AGameCharacter::GiveCharacterAilment(FAilmentInfo info)
{
	if (Role < ROLE_Authority)
		return;

	if (currentAilment.newAilment != EAilment::AL_None)
	{
		ailmentQueue.Enqueue(info);
		return;
	}

	currentAilment = info;

	URealmCharacterMovementComponent* mc = Cast<URealmCharacterMovementComponent>(GetCharacterMovement());
	if (!IsValid(mc))
		return;

	switch (currentAilment.newAilment)
	{
	case EAilment::AL_Knockup: //a knockup is a stun that displaces the character a certain distance.
		mc->AddImpulse(currentAilment.ailmentDir);
		break;

	case EAilment::AL_Stun:
		mc->IgnoreMovementForDuration(currentAilment.ailmentDuration);
		break;
	}

	GetWorldTimerManager().SetTimer(ailmentTimer, this, &AGameCharacter::CurrentAilmentFinished, currentAilment.ailmentDuration);
}

FAilmentInfo AGameCharacter::GetCharacterAilment() const
{
	return currentAilment;
}

void AGameCharacter::CurrentAilmentFinished()
{
	GetWorldTimerManager().ClearTimer(ailmentTimer);

	currentAilment.newAilment = EAilment::AL_None;

	if (!ailmentQueue.IsEmpty())
	{
		FAilmentInfo newInfo;
		ailmentQueue.Dequeue(newInfo);

		if (newInfo.newAilment != EAilment::AL_None)
			GiveCharacterAilment(newInfo);
	}
}

FAilmentInfo AGameCharacter::MakeAilmentInfo(EAilment ailment, FString ailmentString, float ailmentDuration, FVector ailmentDir)
{
	FAilmentInfo info;

	info.newAilment = ailment;
	info.ailmentText = ailmentString;
	info.ailmentDuration = ailmentDuration;
	info.ailmentDir = ailmentDir;

	return info;
}

void AGameCharacter::CalculateVisibility(TArray<AGameCharacter*>& sightList)
{
	ARealmPlayerController* localPC = Cast<ARealmPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!IsValid(localPC))
		return;

	ARealmPlayerState* localPS = Cast<ARealmPlayerState>(localPC->PlayerState);
	if (!IsValid(localPS))
		return;

	TArray<FHitResult> hits;
	FVector start = GetActorLocation();
	FVector end = start;
	end.Z += 5.f;

	GetWorld()->SweepMultiByChannel(hits, start, end, GetActorRotation().Quaternion(), ECC_Visibility, FCollisionShape::MakeSphere(sightRadius));
	for (int32 i = 0; i < hits.Num(); i++)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hits[i].GetActor());
		if (IsValid(gc))
		{
			end = gc->GetActorLocation();
			TArray<FHitResult> tHits;
			GetWorld()->LineTraceMultiByChannel(tHits, start, end, ECC_Visibility);

			for (FHitResult hit : tHits)
			{
				if (hit.GetActor() == gc)
					sightList.AddUnique(gc);
			}
		}
	}
}

bool AGameCharacter::CanEnemyAbsolutelySeeThisUnit() const
{
	return bCanEnemySee;
}

void AGameCharacter::SetEnemyAbsolutelySeeThisUnit(bool bNewSee)
{
	bCanEnemySee = bNewSee;
}

void AGameCharacter::SetGuaranteedCrit(bool bNewCrit /* = false */)
{
	bGuaranteeCrit = bNewCrit;
}

void AGameCharacter::ApplyCharacterAction_Implementation(const FString& actionName, float actionDuration, bool bReverseProgressBar /* = false */, bool bPreventCombat /* = false */)
{
	currentActionName = actionName;

	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(actionTimer, this, &AGameCharacter::CharacterActionFinished, actionDuration);
		bActionPreventingCombat = bPreventCombat;

		if (bActionPreventingCombat)
			StopAutoAttack();
	}
	else
		GetWorldTimerManager().SetTimer(actionTimer, actionDuration, false);
}

void AGameCharacter::CharacterActionFinished()
{
	bActionPreventingCombat = false;
}

void AGameCharacter::HurtAnother(AGameCharacter* hurtCharacter, struct FDamageEvent const& DamageEvent, float damageAmount /* = 0.f */, FRealmDamage const& realmDamage)
{
	if (!IsValid(hurtCharacter) || !IsValid(statsManager) || damageAmount <= 0.f)
		return;

	DamagedOtherCharacter(hurtCharacter, DamageEvent, damageAmount, realmDamage);
	CharacterCombatAction();

	//apply any effects we need to
	//health drain
	if (DamageEvent.DamageTypeClass == UPhysicalDamage::StaticClass() && GetCurrentValueForStat(EStat::ES_HPDrain) > 0.f)
		statsManager->RemoveHealth(damageAmount * (GetCurrentValueForStat(EStat::ES_HPDrain) / 100.f) * -1.f);
}

void AGameCharacter::CharacterCombatAction()
{
	if (Role < ROLE_Authority)
		return;

	if (!bInCombat)
	{
		bInCombat = true;
		CharacterEnteredCombat();
	}

	GetWorldTimerManager().SetTimer(combatTimeout, this, &AGameCharacter::CharacterCombatFinished, combatTimeoutDelay, false);
}

void AGameCharacter::CharacterCombatFinished()
{
	bInCombat = false;
	CharacterLeftCombat();
}

void AGameCharacter::OnUpgradeSkill(int32 index)
{
	if (skillPoints <= 0)
		return;

	ASkillManager* sm = GetSkillManager();
	if (IsValid(sm))
	{
		int32 prevSkill = -1;
		ASkill* skill = sm->GetSkill(index);
		if (IsValid(skill))
		{
			prevSkill = skill->skillPoints;
			skill->AddSkillPoint();
			if (prevSkill != skill->skillPoints)
				skillPoints--;
		}
	}
}

void AGameCharacter::PlayCharacterSound_Implementation(USoundBase* sound, bool bAttachedToCharacter /* = false */)
{
	if (!sound || bHidden)
		return;

	if (bAttachedToCharacter)
		UGameplayStatics::SpawnSoundAttached(sound, GetRootComponent(), NAME_None, FVector(ForceInit), EAttachLocation::KeepRelativeOffset, false, 0.5f, 1.f, 0.f, soundAttenuation);
	else
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), sound, GetActorLocation(), FRotator::ZeroRotator, 0.5f, 1.f, 0.f, soundAttenuation);
}

void AGameCharacter::AddShield(FCharacterShield newShield)
{
	if (IsValid(shieldManager))
		shieldManager->AddShield(newShield);
}

void AGameCharacter::AddSpecificDamager(AGameCharacter* specificCharacter)
{
	specificDamagingCharacters.AddUnique(specificCharacter);
}

void AGameCharacter::SetOnlySpecificDamage(bool bNewSpecificDamage)
{
	bOnlySpecificCharactersCanDamage = bNewSpecificDamage;
}

void AGameCharacter::ClearSpecificDamagers()
{
	specificDamagingCharacters.Empty();
}

FVector AGameCharacter::FindGroundBeneathCharacter(AGameCharacter* testCharacter)
{
	if (!IsValid(testCharacter))
		return FVector::ZeroVector;

	FVector start = testCharacter->GetActorLocation();
	FVector end = start;
	end.Z -= 10000.f;

	FHitResult hit;
	if (testCharacter->GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic))
		return hit.ImpactPoint;
	else
		return testCharacter->GetActorLocation();
}

bool AGameCharacter::CanSeeOtherCharacter(AGameCharacter* testCharacter)
{
	if (!IsValid(testCharacter))
		return false;

	//test to see if the test character is in our vision range and we have line of sight to them
	if (testCharacter->CanEnemyAbsolutelySeeThisUnit() || testCharacter->GetTeamIndex() == teamIndex)
		return true;

	FVector start = GetActorLocation();
	FVector end = testCharacter->GetActorLocation();

	float dist = (start - end).Size2D();
	if (dist > sightRadius)
		return false;

	TArray<FHitResult> tHits;
	GetWorld()->LineTraceMultiByChannel(tHits, start, end, ECC_Visibility);

	for (FHitResult hit : tHits)
	{
		if (hit.GetActor() == testCharacter)
			return true;
	}

	return false;
}

//----------------------------------------------------------REPLICATION FUNCTIONS----------------------------------------------------------

bool AGameCharacter::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	/*const ARealmPlayerController* player = Cast<ARealmPlayerController>(RealViewer);

	if (IsValid(player) && IsValid(player->fogOfWar) && player->fogOfWar->enemySightList.Num() > 0)
		return player->fogOfWar->enemySightList.Contains(this); //only be relevant to players who see this unit*/

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void AGameCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(AGameCharacter, lastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < lastTakeHitTimeTimeout);
}

bool AGameCharacter::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (autoAttackManager)
		WroteSomething |= Channel->ReplicateSubobject(autoAttackManager, *Bunch, *RepFlags);

	if (statsManager)
		WroteSomething |= Channel->ReplicateSubobject(statsManager, *Bunch, *RepFlags);

	if (modManager)
		WroteSomething |= Channel->ReplicateSubobject(modManager, *Bunch, *RepFlags);

	return WroteSomething;
}

void AGameCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameCharacter, statsManager);
	DOREPLIFETIME(AGameCharacter, autoAttackManager);
	DOREPLIFETIME(AGameCharacter, skillManager);
	DOREPLIFETIME(AGameCharacter, shieldManager);
	DOREPLIFETIME(AGameCharacter, teamIndex);
	DOREPLIFETIME(AGameCharacter, currentAilment);
	DOREPLIFETIME(AGameCharacter, currentTarget);
	DOREPLIFETIME(AGameCharacter, level);
	DOREPLIFETIME(AGameCharacter, skillPoints);
	DOREPLIFETIME(AGameCharacter, experienceAmount);
	DOREPLIFETIME(AGameCharacter, mods);
	DOREPLIFETIME(AGameCharacter, bIsTargetable);
	DOREPLIFETIME(AGameCharacter, bAutoAttackLaunching);
	DOREPLIFETIME_CONDITION(AGameCharacter, lastTakeHitInfo, COND_Custom);
}
