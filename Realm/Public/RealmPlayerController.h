#pragma once

#include "RealmPlayerController.generated.h"

class ARealmMoveController;
class APlayerCharacter;

UCLASS()
class ARealmPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

protected:

	/* timer that calls a movement update function on the server */
	UPROPERTY()
	FTimerHandle movementTimer;

	/* timer for getting in range for auto attacks */
	UPROPERTY()
	FTimerHandle aaRangeTimer;

	/* particle system for the move command */
	UPROPERTY()
	UParticleSystem* moveParticleSystem;

	/* move controller that handles all of the pathing for the player */
	UPROPERTY()
	ARealmMoveController* moveController;

	/* temporary until character select is implemented */
	UPROPERTY()
	TSubclassOf<APlayerCharacter> defaultCharacterClass;

	/* reference to the player character this player is using */
	UPROPERTY(replicated)
	APlayerCharacter* playerCharacter;

	/* whether or not we have the ingame store open */
	bool bIngameStoreOpen;

	/* override begin play */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/* [CLIENT] open the player's in-game store */
	void OnOpenIngameStore();

	/* [CLIENT] open the player's in-game store */
	void OnCloseIngameStore();

	/* [CLIENT] toggle the players store */
	void OnToggleIngameStore();

	/* [CLIENT] player selects a target */
	void OnTargetSelect();

public:

	/* [SERVER] calls the server to send the calculated world position to the move controller */
	UFUNCTION(reliable, server, WithValidation)
	void ServerMoveCommand(FVector targetLocation);

	/* [SERVER] start aa cycle */
	UFUNCTION(reliable, server, WithValidation)
	void ServerStartAutoAttack(AGameCharacter* target);

	/* [SERVER] clear any command timers and stop movement */
	UFUNCTION(reliable, server, WithValidation)
	void ServerClearMoveCommands();

	/* [SERVER] clear any command timers and stop auto attacks */
	UFUNCTION(reliable, server, WithValidation)
	void ServerClearAttackCommands();

	/* [SERVER] called when the player wants to use a skill */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUseSkill(int32 index, FVector mouseHitLoc);

	/* [SERVER] sets the chosen character class */
	UFUNCTION(reliable, server, WithValidation, BlueprintCallable, Category=Character)
	void ServerChooseCharacter(TSubclassOf<APlayerCharacter> chosenCharacter);

	/* [CLIENT] initialize the player's in-game store */
	UFUNCTION(reliable, client)
	void ClientInitIngameStore(const TArray<TSubclassOf<AMod> >& modStore);

	/* [CLIENT] open pregame screen */
	UFUNCTION(reliable, client)
	void ClientOpenPregameScreen();

	/* [CLIENT] open character select */
	UFUNCTION(reliable, client)
	void ClientOpenCharacterSelectScreen(const TArray<TSubclassOf<APlayerCharacter> >& availableCharacters);

	/* [CLIENT] initialize and show the player hud */
	UFUNCTION(reliable, client)
	void ClientOpenPlayerHUD();

	/* [CLIENT] try to buy a mod */
	UFUNCTION(reliable, server, WithValidation)
	void ServerBuyPlayerMod(TSubclassOf<AMod> wantedMod);

	/* [CLIENT] try to sell a mod */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSellPlayerMod(int32 index);

	/* gets the player character in range for auto attacks */
	void GetPlayerInAutoAttackRange();

	/* gets the class of player character this player wants to spawn with */
	UFUNCTION(BlueprintCallable, Category=PC)
	TSubclassOf<APlayerCharacter> GetDefaultCharacterClass() const;

	/* get the player character */
	UFUNCTION(BlueprintCallable, Category = PC)
	APlayerCharacter* GetPlayerCharacter() const;

	/* override this to call the move controller's possess, if it exists */
	virtual void Possess(APawn* aPawn) override;

	/* get the move controller */
	ARealmMoveController* GetMoveController() const;
};