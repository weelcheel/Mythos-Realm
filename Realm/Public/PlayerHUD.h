#pragma once

#include "DamageInstance.h"
#include "PlayerHUD.generated.h"

class AGameCharacter;
class ARealmPlayerState;

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

	/* notifies this HUD about a player kill */
	UFUNCTION(BlueprintImplementableEvent, Category = Announcements)
	void NotifyPlayerKill(ARealmPlayerState* killer, ARealmPlayerState* killed, APawn* killerPawn, bool bIsThisPlayerDead = false, bool bIsThisPlayerKiller = false);

	/* notifies this HUD about a player kill */
	UFUNCTION(BlueprintImplementableEvent, Category = Announcements)
	void NotifyObjectiveKill(APawn* killerPawn, ARealmObjective* objectiveDestroyed);

	/* server has sent out the game over message */
	UFUNCTION(BlueprintImplementableEvent, Category = Postgame)
	void NotifyEndGame(int32 teamVictor);
};