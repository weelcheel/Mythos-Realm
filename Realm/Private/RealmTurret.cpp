#include "Realm.h"
#include "RealmTurret.h"
#include "RealmTurretAI.h"
#include "MinionCharacter.h"
#include "PlayerCharacter.h"

ATurret::ATurret(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	bCanAffectNavigationGeneration = true;
	GetCapsuleComponent()->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);

	AIControllerClass = ARealmTurretAI::StaticClass();

	bReplicateMovement = false;

	NetUpdateFrequency = 10.f;
}

void ATurret::OnSeePawn(APawn* OtherActor)
{
	AGameCharacter* gc = Cast<AGameCharacter>(OtherActor);

	if (gc && (!gc->IsAlive() || gc->GetTeamIndex() == GetTeamIndex()))
		return;

	if (!currentTarget && IsValid(gc))
	{
		SetCurrentTarget(gc);
		StartAutoAttack();
	}
}

void ATurret::CheckAutoAttack()
{
	if (!IsValid(currentTarget) || !IsValid(GetController()) || !currentTarget->IsAlive() || !currentTarget->IsTargetable())
	{
		StopAutoAttack();
		TargetOutofRange();
		return;
	}

	float distance = (GetActorLocation() - currentTarget->GetActorLocation()).Size2D();
	if (distance > statsManager->GetCurrentValueForStat(EStat::ES_AARange))
	{
		StopAutoAttack();
		TargetOutofRange();
	}
	else if (!bAutoAttackLaunching && !bAutoAttackOnCooldown)
	{
		float scale = statsManager->GetCurrentValueForStat(EStat::ES_AtkSp) / statsManager->GetBaseValueForStat(EStat::ES_AtkSp);
		//AllPlayAnimMontage(autoAttackManager->GetCurrentAttackAnimation(), scale);

		GetWorldTimerManager().SetTimer(aaLaunchTimer, this, &AGameCharacter::LaunchAutoAttack, autoAttackManager->GetAutoAttackLaunchTime() / scale);

		bAutoAttackLaunching = true;
	}
}

void ATurret::OnFinishAATimer()
{
	bAutoAttackOnCooldown = false;
	bAutoAttackLaunching = false;

	if (IsValid(GetCurrentTarget()) && GetCurrentTarget()->IsAlive() && !GetCurrentTarget()->bHidden)
		StartAutoAttack();
	else
		TargetOutofRange();
}

void ATurret::TargetOutofRange()
{
	if (!IsValid(this))
		return;

	TArray<FHitResult> hits;
	FVector start = GetActorLocation();
	FVector end = start;
	end.Z += 5.f;

	TArray<AGameCharacter*> possibleTargets;

	GetWorld()->SweepMultiByChannel(hits, start, end, GetActorRotation().Quaternion(), ECC_Visibility, FCollisionShape::MakeSphere(GetCurrentValueForStat(EStat::ES_AARange)));
	for (FHitResult hit : hits)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
		if (IsValid(gc) && gc->IsAlive() && GetTeamIndex() != gc->GetTeamIndex())
			possibleTargets.AddUnique(gc);
	}

	//first aggro any minions first
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(AMinionCharacter::StaticClass()))
		{
			SetCurrentTarget(gc);
			StartAutoAttack();

			return;
		}
	}

	//then aggro an objective if we can 
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(ARealmObjective::StaticClass()))
		{
			SetCurrentTarget(gc);
			StartAutoAttack();

			return;
		}
	}

	//lastly aggro any mythos
	for (AGameCharacter* gc : possibleTargets)
	{
		if (gc->IsA(APlayerCharacter::StaticClass()))
		{
			SetCurrentTarget(gc);
			StartAutoAttack();

			return;
		}
	}

	StopAutoAttack();
}

/*void ATurret::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC))
		return;

	if (bHidden)
		return;

	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D(150.f, 8.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 150.f, 8.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
	}
}*/

void ATurret::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{
	if (distressedUnit->IsA(APlayerCharacter::StaticClass()) && enemyTarget->IsA(APlayerCharacter::StaticClass()) && currentTarget != enemyTarget)
	{
		currentTarget = enemyTarget;
		//ResetAutoAttack();
	}
}

void ATurret::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage, damageDesc);

	if (Role == ROLE_Authority)
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->ObjectiveDestroyed(this, InstigatingPawn);
}