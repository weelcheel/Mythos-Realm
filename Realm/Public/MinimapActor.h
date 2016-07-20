#pragma once

#include "MinimapActor.generated.h"

class APlayerHUD;

USTRUCT(BlueprintType)
struct FMinimapEntry
{
	GENERATED_USTRUCT_BODY()

	/* character for this minimap entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Minimap)
	AGameCharacter* character;

	/* relative position for this character to the minimap's origins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Minimap)
	FVector2D relativePosition;
};

UCLASS()
class AMinimapActor : public AActor
{
	friend class APlayerHUD;

	GENERATED_UCLASS_BODY()

protected:

	/* size of the outside boundaries of this map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Size)
	FVector2D mapSizeMin;

	/* size of the outside boundaries of this map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Size)
	FVector2D mapSizeMax;

	/* center of the map in the map texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Size)
	FVector2D mapCenter;

	/* sphere component to get the bounds of the map */
	UPROPERTY(EditAnywhere, Category = Size)
	USphereComponent* mapExtents;

public:

	/* hud this minimap is being used for */
	UPROPERTY()
	APlayerHUD* hud;

	/* texture that UI's can use to draw for the background of the minimap */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Minimap)
	UMaterialInstanceConstant* mapBackground;

	/* updates visible characters for the minimap */
	void ReceiveCharacterVisibilityUpdate(const TArray<AGameCharacter*>& visibleCharacters);

	/* gets the radian heading for this minimap actor */
	float GetRadianHeading() const;

	/* gets the degree heading for the minimap actor */
	float GetDegreeHeading() const;

	/* begin play for variable setup */
	void BeginPlay() override;
};