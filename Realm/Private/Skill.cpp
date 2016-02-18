#include "Realm.h"
#include "Skill.h"
#include "GameCharacter.h"
#include "UnrealNetwork.h"

ASkill::ASkill(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	skillState = ESkillState::NoOwner;
	bAutoPerform = true;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
}

float ASkill::SkillLevelScale(float min, float max, bool bIncreasing) const
{
	if (skillPoints <= 0)
		return 0.f;

	float interval = (max - min) / skillPointsMax;
	float scaledValue;

	if (bIncreasing)
		scaledValue = min + (interval*(skillPoints - 1));
	else
		scaledValue = max - (interval*(skillPoints - 1));

	return scaledValue;
}

FDamageEvent ASkill::CreateDamageEvent(TSubclassOf<UDamageType> damageType)
{
	FDamageEvent damageEvent(damageType);
	
	return damageEvent;
}

bool ASkill::SphereTrace(AActor* actorToIgnore, const FVector& start, const FVector& end, const float radius, TArray<FHitResult>& hitOut, ECollisionChannel traceChannel /* = ECC_Pawn */)
{
	FCollisionQueryParams traceParams(FName(TEXT("Sphere Trace")), true, actorToIgnore);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	traceParams.AddIgnoredActor(actorToIgnore);

	TObjectIterator<APlayerController> pc;
	if (!pc)
		return false;

	DrawDebugSphere(pc->GetWorld(), start, radius, 8, FColor::Red, true);

	return pc->GetWorld()->SweepMultiByChannel(hitOut, start, end, FQuat(), traceChannel, FCollisionShape::MakeSphere(radius), traceParams);
}

bool ASkill::ConeTrace(AActor* actorToIgnore, const FVector& start, const FVector& dir, float coneHeight, TArray<AGameCharacter*>& hitsOut, ECollisionChannel traceChannel /* = ECC_Pawn */)
{
	TArray<FHitResult> sphereHits1, sphereHits2, sphereHits3, sphereHits4;
	FVector dp(0.f, 0.0f, 1.0f);

	FVector p1 = start + (dir * (0.045*coneHeight));
	SphereTrace(actorToIgnore, p1, p1 + dp, 0.045f*coneHeight, sphereHits1, traceChannel);

	FVector p2 = start + (dir * (0.17*coneHeight));
	SphereTrace(actorToIgnore, p2, p2 + dp, 0.08f*coneHeight, sphereHits2, traceChannel);

	FVector p3 = start + (dir * (0.395*coneHeight));
	SphereTrace(actorToIgnore, p3, p3 + dp, 0.145f*coneHeight, sphereHits3, traceChannel);

	FVector p4 = start + (dir * (0.77*coneHeight));
	SphereTrace(actorToIgnore, p4, p4 + dp, 0.23f*coneHeight, sphereHits4, traceChannel);

	sphereHits1.Append(sphereHits2);
	sphereHits1.Append(sphereHits3);
	sphereHits1.Append(sphereHits4);

	for (int32 i = 0; i < sphereHits1.Num(); i++)
	{
		AGameCharacter* mc = Cast<AGameCharacter>(sphereHits1[i].GetActor());
		if (mc)
			hitsOut.AddUnique(mc);
	}

	return true;
}

void ASkill::InitializeSkill(AGameCharacter* owner)
{
	if (Role < ROLE_Authority)
		return;

	SetOwner(owner->GetController());

	characterOwner = owner;
	skillState = ESkillState::NotLearned;
}

void ASkill::AddSkillPoint()
{
	if (Role < ROLE_Authority)
		return;

	if (skillState == ESkillState::NotLearned)
		skillState = ESkillState::Ready;

	if (skillPoints + 1 <= skillPointsMax)
		skillPoints++;
}

float ASkill::GetCooldownProgressPercent()
{
	if (skillState != ESkillState::OnCooldown)
		return 0.f;

	float timeElapsed = GetWorldTimerManager().GetTimerElapsed(cooldownTimer);
	float totalTime = timeElapsed + GetWorldTimerManager().GetTimerRemaining(cooldownTimer);

	return timeElapsed / totalTime;
}

float ASkill::GetCooldownRemaining()
{
	return GetWorldTimerManager().GetTimerRemaining(cooldownTimer);
}

void ASkill::StartCooldown()
{
	skillState = ESkillState::OnCooldown;

	cooldownTime = SkillLevelScale(cooldownMin, cooldownMax, false);
	GetWorldTimerManager().SetTimer(cooldownTimer, this, &ASkill::CooldownFinished, cooldownTime);
}

void ASkill::CooldownFinished()
{
	skillState = ESkillState::Ready;
	cooldownTime = 0.f;
}

void ASkill::OnCooldownTimerSet()
{
	GetWorldTimerManager().SetTimer(cooldownTimer, this, &ASkill::CooldownFinished, cooldownTime);
}

void ASkill::SkillFinished()
{
	StartCooldown();
}

float ASkill::GetCost()
{
	return cost;
}

ESkillState ASkill::GetSkillState() const
{
	return skillState;
}

void ASkill::SetSkillState(ESkillState newState)
{
	skillState = newState;
}

void ASkill::ServerSkillPerformed_Implementation(FVector mouseHitLoc, AGameCharacter* targetUnit /* = NULL */)
{
	characterOwner->UseFlare(cost);
}

void ASkill::InterruptSkill(ESkillInterruptReason interruptReason)
{
	if (skillState != ESkillState::Performing)
		return;

	OnCanInterruptSkill(interruptReason);
}

void ASkill::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASkill, characterOwner);
	DOREPLIFETIME(ASkill, skillPoints);
	DOREPLIFETIME(ASkill, skillState);
	DOREPLIFETIME(ASkill, cooldownTime);
}