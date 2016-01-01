#pragma once

#include "DamageInstance.h"
#include "PlayerHUD.generated.h"

class AGameCharacter;

USTRUCT()
struct FUIDamage
{
	GENERATED_USTRUCT_BODY()

	float amount;
	float originTime;
	FVector worldPosition;
	TSubclassOf<UDamageType> damageType;
	float posY;
};

UCLASS(Blueprintable)
class APlayerHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

protected:

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	TArray<FUIDamage> floatingDamage;

	UPROPERTY()
	UFont* uiFont;

public:

	void NewDamageEvent(FTakeHitInfo hitInfo, FVector worldPosition);

	UFUNCTION(BlueprintImplementableEvent, Category = Store)
	void InitIngameStore(const TArray<TSubclassOf<AMod> >& modStore);

	UFUNCTION(BlueprintImplementableEvent, Category=Store)
	void OpenInGameStore();

	UFUNCTION(BlueprintImplementableEvent, Category = Store)
	void CloseInGameStore();

	UFUNCTION(BlueprintImplementableEvent, Category = Pregame)
	void OpenPregameScreen();

	UFUNCTION(BlueprintImplementableEvent, Category = Pregame)
	void OpenCharacterSelectScreen(const TArray<TSubclassOf<APlayerCharacter> >& availableCharacters);

	UFUNCTION(BlueprintImplementableEvent, Category = Pregame)
	void OpenPlayerHUD();

	UFUNCTION(BlueprintImplementableEvent, Category = Effects)
	void CharacterEffectsUpdated(AGameCharacter* updatedCharacter);
};