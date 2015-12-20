#include "Realm.h"
#include "Projectile.h"
#include "GameCharacter.h"
#include "UnrealNetwork.h"

AProjectile::AProjectile(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	collisionComp = objectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	collisionComp->InitSphereRadius(5.0f);
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
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(100.f);

	if (IsValid(homingTarget) && !homingTarget->IsAlive())
		Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::InitializeProjectile(const FVector& AimDir, float inDamage, TSubclassOf<UDamageType> projDamage, AGameCharacter* projSpawner /* = nullptr */, AGameCharacter* projTarget /* = nullptr */)
{
	movementComponent->Velocity = AimDir * movementComponent->InitialSpeed;
	damage = inDamage;
	damageType = projDamage;
	projectileSpawner = projSpawner;

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

	ServerProjectileCollision();

	if (damage <= 0.f || damageType == nullptr)
		return;
		
	if (homingTarget && OtherActor == homingTarget)
	{
		FDamageEvent damageEvent(damageType);
		homingTarget->TakeDamage(damage, damageEvent, projectileSpawner->GetRealmController(), this);
		projectileSpawner->DamagedOtherCharacter(homingTarget, damage, bAutoAttackProjectile);

		Destroy();
	}
	else if (!homingTarget)
	{
		AGameCharacter* gc = Cast<AGameCharacter>(OtherActor);
		if (!IsValid(gc)) //its not a game character so we don't collide
			return;

		if (!damageType)
			return;

		FDamageEvent damageEvent(damageType);
		gc->TakeDamage(damage, damageEvent, projectileSpawner->GetRealmController(), this);
		projectileSpawner->DamagedOtherCharacter(gc, damage, bAutoAttackProjectile);

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
}