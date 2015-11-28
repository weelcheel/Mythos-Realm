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
}

void ATurret::OnSeePawn(APawn* OtherActor)
{
	AGameCharacter* gc = Cast<AGameCharacter>(OtherActor);

	if (gc && (!gc->IsAlive() || gc->GetTeamIndex() == GetTeamIndex()))
		return;

	if (currentTarget && gc)
		inRangeTargets.AddUnique(gc);
	else if (!currentTarget && gc)
	{
		currentTarget = gc;
		StartAutoAttack();
	}
}

void ATurret::CheckAutoAttack()
{
 	if (!IsValid(currentTarget) || !IsValid(GetController()) || !currentTarget->IsAlive())
	{
		TargetOutofRange();

		return;
	}

	float distanceSq = (GetActorLocation() - currentTarget->GetActorLocation()).SizeSquared2D();
	if (distanceSq > autoAttackManager->GetCurrentAutoAttackRangeSquared())
	{
		StopAnimMontage();
		GetWorldTimerManager().ClearTimer(aaLaunchTimer);

		TargetOutofRange();

		bAutoAttackLaunching = false;
	}
}

void ATurret::TargetOutofRange()
{
	float leastDistance = -1.f;
	int32 leastInd = -1;

	for (int32 i = 0; i < inRangeTargets.Num(); i++)
	{
		if (!IsValid(inRangeTargets[i]))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (!inRangeTargets[i]->IsA(AMinionCharacter::StaticClass()))
			continue;

		float distanceSq = (inRangeTargets[i]->GetActorLocation() - GetActorLocation()).SizeSquared2D();
		if (!inRangeTargets[i]->IsAlive() || distanceSq > FMath::Square(710.f))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (leastDistance == -1.f)
		{
			leastDistance = distanceSq;
			leastInd = i;
			continue;
		}

		if (distanceSq < leastDistance)
		{
			leastDistance = distanceSq;
			leastInd = i;
		}
	}

	if (leastInd >= 0)
	{
		currentTarget = inRangeTargets[leastInd];
		return;
	}

	for (int32 i = 0; i < inRangeTargets.Num(); i++)
	{
		if (!IsValid(inRangeTargets[i]))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (inRangeTargets[i]->IsA(AMinionCharacter::StaticClass()))
			continue;

		float distanceSq = (inRangeTargets[i]->GetActorLocation() - GetActorLocation()).SizeSquared2D();
		if (!inRangeTargets[i]->IsAlive() || distanceSq > FMath::Square(710.f))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (leastDistance == -1.f)
		{
			leastDistance = distanceSq;
			leastInd = i;
			continue;
		}

		if (distanceSq < leastDistance)
		{
			leastDistance = distanceSq;
			leastInd = i;
		}
	}

	if (leastInd >= 0)
	{
		currentTarget = inRangeTargets[leastInd];
	}
	else
	{
		currentTarget = nullptr;
		StopAutoAttack();
	}
}

void ATurret::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC->GetCharacter()))
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
}

void ATurret::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{
	if (distressedUnit->IsA(APlayerCharacter::StaticClass()) && enemyTarget->IsA(APlayerCharacter::StaticClass()))
		currentTarget = enemyTarget;
}