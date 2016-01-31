#pragma once

#include "DamageInstance.h"
#include "DamageTypes.h"
#include "PlayerHUD.generated.h"

class AGameCharacter;
class ARealmPlayerState;

USTRUCT()
struct FUIDamage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float originTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector worldPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UDamageType> damageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FRealmDamage realmDamage;
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

	void NewDamageEvent(FTakeHitInfo hitInfo, FVector worldPosition, FRealmDamage& realmdmg);

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

	/* called when the hud needs to display damage text */
	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void ShowDamageText(FUIDamage dmg);

	//Float as FText With Precision!
	UFUNCTION(BlueprintCallable, Category=FloatPrecision)
	static FText GetFloatAsTextWithPrecision(float TheFloat, int32 Precision, bool IncludeLeadingZero = true)
	{
		FNumberFormattingOptions NumberFormat;					//Text.h
		NumberFormat.MinimumIntegralDigits = (IncludeLeadingZero) ? 1 : 0;
		NumberFormat.MaximumIntegralDigits = 10000;
		NumberFormat.MinimumFractionalDigits = Precision;
		NumberFormat.MaximumFractionalDigits = Precision;
		return FText::AsNumber(TheFloat, &NumberFormat);
	}
};