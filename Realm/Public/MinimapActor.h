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
	GENERATED_UCLASS_BODY()

protected:

	/* size of the outside boundaries of this map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Size)
	FVector2D mapSize;

public:

	/* hud this minimap is being used for */
	UPROPERTY()
	APlayerHUD* hud;

	/* texture that UI's can use to draw for the background of the minimap */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Minimap)
	UTexture2D* mapBackground;

	/* updates visible characters for the minimap */
	void ReceiveCharacterVisibilityUpdate(const TArray<AGameCharacter*>& visibleCharacters);
};