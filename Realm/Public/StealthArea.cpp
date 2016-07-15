#include "Realm.h"
#include "StealthArea.h"
#include "GameCharacter.h"

AStealthArea::AStealthArea(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	areaCollision = objectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("areaCollision"));
	staticMeshComponent = objectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("areaMesh"));

	areaCollision->OnComponentBeginOverlap.AddDynamic(this, &AStealthArea::OnComponentOverlap);
	areaCollision->OnComponentEndOverlap.AddDynamic(this, &AStealthArea::OnComponentEndOverlap);
	RootComponent = areaCollision;

	//default setup is just box collision; can change, however, in editor for specific instances of areas (like a tri-bush for example should have mesh based collision instead)
	areaCollision->SetCollisionObjectType(ECC_Visibility);
	areaCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	//areaCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	areaCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	staticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	bReplicates = true;
	bReplicateMovement = false;
	NetUpdateFrequency = 30.f;
}

void AStealthArea::OnComponentOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGameCharacter* gc = Cast<AGameCharacter>(OtherActor);
	if (IsValid(gc))
		AddOccupyingUnit(gc);
}

void AStealthArea::OnComponentEndOverlap(AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AGameCharacter* gc = Cast<AGameCharacter>(Other);
	if (IsValid(gc))
		RemoveOccupyingUnit(gc);
}

void AStealthArea::AddOccupyingUnit(AGameCharacter* newUnit)
{
	newUnit->currentStealthArea = this;
	occupyingUnits.AddUnique(newUnit);
}

void AStealthArea::RemoveOccupyingUnit(AGameCharacter* exitingUnit)
{
	exitingUnit->currentStealthArea = nullptr;
	occupyingUnits.Remove(exitingUnit);
}

void AStealthArea::CalculateVisibility(AGameCharacter* calculatingUnit, TArray<AGameCharacter*>& sightList, TArray<AGameCharacter*>& availableUnits)
{
	if (!IsValid(calculatingUnit))
		return;

	//first get visible units on the outside based on increased radial area
	TArray<AActor*> ignoreList;
	for (AGameCharacter* gc : availableUnits)
		ignoreList.AddUnique(gc);

	for (AGameCharacter* gc : availableUnits)
	{
		if (occupyingUnits.Contains(gc)) //don't account for units in this area yet
			continue;

		FVector start = GetActorLocation();
		FVector end = gc->GetActorLocation();

		if ((start - end).SizeSquared2D() <= FMath::Square(calculatingUnit->sightRadius * 1.5f))
		{
			FHitResult hit;
			FCollisionQueryParams collisionParams;

			TArray<AActor*> ignoredActors = ignoreList;
			ignoredActors.Remove(gc);

			collisionParams.AddIgnoredActors(ignoredActors);
			collisionParams.AddIgnoredActor(this);

			GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility, collisionParams);

			if (hit.GetActor() == gc)
			{
				if (!IsValid(gc->currentStealthArea))
					sightList.AddUnique(gc);
			}
		}
	}

	//finally add all of the units that are currently in the area
	for (AGameCharacter* gc : occupyingUnits)
	{
		if (IsValid(gc))
			sightList.AddUnique(gc);
		else
			sightList.Remove(gc);
	}
}