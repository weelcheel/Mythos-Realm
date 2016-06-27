#include "Realm.h"
#include "Projectile.h"
#include "GameCharacter.h"
#include "UnrealNetwork.h"
#include "RealmPlayerController.h"

AProjectile::AProjectile(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	collisionComp = objectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	collisionComp->InitSphereRadius(15.0f);
	collisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnHit);
	collisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	collisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	collisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	collisionComp->CanCharacterStepUpOn = ECB_No;
	collisionComp->SetCanEverAffectNavigation(false);

	RootComponent = collisionComp;

	movementComponent = objectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	movementComponent->UpdatedComponent = collisionComp;
	movementComponent->InitialSpeed = 1000.f;
	movementComponent->MaxSpeed = 1000.f;
	movementComponent->bRotationFollowsVelocity = true;
	movementComponent->bShouldBounce = false;
	movementComponent->HomingAccelerationMagnitude = 50000.f;

	bReplicates = true;
	bReplicateMovement = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;

	NetUpdateFrequency = 30.f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(25.f);

	if (IsValid(homingTarget) && !homingTarget->IsAlive())
		Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		if ((movementComponent->bIsHomingProjectile && !IsValid(homingTarget)) || (movementComponent->bIsHomingProjectile && IsValid(homingTarget) && !homingTarget->IsTargetable()))
		{
			Destroy();
			return;
		}

		if ((!IsValid(projectileSpawner) || (movementComponent->bIsHomingProjectile && IsValid(homingTarget) && !homingTarget->IsAlive())) && GetWorldTimerManager().GetTimerRemaining(TimerHandle_LifeSpanExpired) <= 0.f)
		{
			SetLifeSpan(5.f);
			return;
		}
	}
	else
	{
		//if its a homing projectile, hide if the owner is hidden unless the target is the local player
		if (IsValid(homingTarget) && IsValid(projectileSpawner) && projectileSpawner->bHidden)
			SetActorHiddenInGame(true);

		ARealmPlayerController* localPC = Cast<ARealmPlayerController>(GetWorld()->GetFirstPlayerController());
		if (bHidden && IsValid(localPC) && IsValid(localPC->GetPlayerCharacter()))
			SetActorHiddenInGame(!(localPC->GetPlayerCharacter() == homingTarget));
	}
}

void AProjectile::InitializeProjectile(const FVector& AimDir, float inDamage, TSubclassOf<UDamageType> projDamage, AGameCharacter* projSpawner /* = nullptr */, AGameCharacter* projTarget /* = nullptr */, FRealmDamage const& rdmg, float spdScale)
{
	movementComponent->Velocity = AimDir * movementComponent->InitialSpeed;
	damage = inDamage;
	damageType = projDamage;
	projectileSpawner = projSpawner;
	realmDamage = rdmg;
	movementComponent->InitialSpeed *= spdScale;
	movementComponent->MaxSpeed *= spdScale;

	if (IsValid(projTarget))
	{
		homingTarget = projTarget;
		movementComponent->HomingTargetComponent = homingTarget->GetRootComponent();
		movementComponent->bIsHomingProjectile = true;
	}
}

void AProjectile::OnHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Role < ROLE_Authority)
	{
		ClientProjectileCollision();
		return;
	}

	ServerProjectileCollision(Cast<AGameCharacter>(OtherActor));

	if (damageType == nullptr)
		return;
		
	if (IsValid(homingTarget) && OtherActor == homingTarget && IsValid(projectileSpawner))
	{
		homingTarget->PlayCharacterSound(hitSound);

		FDamageEvent damageEvent(damageType);
		homingTarget->CharacterTakeDamage(damage, damageEvent, projectileSpawner->GetRealmController(), this, realmDamage, damageDesc);

		Destroy();
	}
	else if (!homingTarget)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(OtherActor);
		if (!IsValid(gc)) //its not a game character so we don't collide
			return;

		if (!damageType)
			return;

		//play hit sound
		gc->PlayCharacterSound(hitSound);

		FDamageEvent damageEvent(damageType);
		//gc->TakeDamage(damage, damageEvent, projectileSpawner->GetRealmController(), this);
		gc->CharacterTakeDamage(damage, damageEvent, projectileSpawner->GetRealmController(), this, realmDamage, damageDesc);

		Destroy();
	}
}

void AProjectile::OnRep_HomingTarget()
{
	if (IsValid(homingTarget))
	{ 
		movementComponent->HomingTargetComponent = homingTarget->GetRootComponent();
		movementComponent->bIsHomingProjectile = true;
	}
}

void AProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile, homingTarget);
	DOREPLIFETIME(AProjectile, hitSound);
}