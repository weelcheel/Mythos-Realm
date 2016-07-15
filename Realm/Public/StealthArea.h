#pragma once

#include "StealthArea.generated.h"

class AGameCharacter;

UCLASS()
class AStealthArea : public AActor
{
	GENERATED_BODY()

protected:

	/* collision area component */
	UPROPERTY(EditAnywhere, Category = Collision)
	UBoxComponent* areaCollision;

	/* static mesh to represent the area */
	UPROPERTY(EditAnywhere, Category = Mesh)
	UStaticMeshComponent* staticMeshComponent;

	/* list of units that are currently occupying this area */
	UPROPERTY(BlueprintReadOnly, Category = Units)
	TArray<AGameCharacter*> occupyingUnits;

	/* called whenever a unit overlaps with this stealth area */
	UFUNCTION()
	void OnComponentOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/* called whenever a unit stops overlapping with the stealth area */
	UFUNCTION()
	void OnComponentEndOverlap(AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	AStealthArea(const FObjectInitializer& objectInitializer);

	/* called by the units that are occupying this area to get the enhanced vision */
	void CalculateVisibility(AGameCharacter* calculatingUnit, TArray<AGameCharacter*>& sightList, TArray<AGameCharacter*>& availableUnits);

	/* removes a unit from the occupyingUnits list */
	void RemoveOccupyingUnit(AGameCharacter* exitingUnit);

	/* adds a unit to the occupyingUnits list */
	void AddOccupyingUnit(AGameCharacter* newUnit);
};