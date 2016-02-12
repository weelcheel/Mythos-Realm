#pragma once

#include "RealmCharacter.h"
#include "SpectatorCharacter.generated.h"

UCLASS()
class ASpectatorCharacter : public ARealmCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* local variable on whether or not this character is holding down the move command button */
	UPROPERTY()
	bool bMoveCommand;

	/* whether or not were locked on a character */
	UPROPERTY()
	bool bLockedOnCharacter;

	/* actor were locked on */
	AActor* cameraLockTarget;

	UPROPERTY()
	FTimerHandle movementTimer;

	/* camera boom */
	UPROPERTY(EditDefaultsOnly, Category = Camera)
	USpringArmComponent* springArm;

	/* rts camera */
	UPROPERTY(EditDefaultsOnly, Category = Camera)
	UCameraComponent* rtsCamera;

	FTimerHandle lockTimer;

	/* speed that the spectator camera travels at */
	UPROPERTY()
	float camSpeed;

	virtual void BeginPlay() override;

	/* override input setup */
	virtual void SetupPlayerInputComponent(class UInputComponent* InInputComponent) override;

	/* [CLIENT] function called when the player holds down the move button */
	void OnDirectedMoveStart();

	/* [CLIENT] function called when the player stops holding down the move button */
	void OnDirectedMoveStop();

	/* on skill used */
	void OnUseSkill(int32 index);

	/* on skill used finished */
	void OnUseSkillFinished(int32 index);

	/* zoom in the camera */
	void OnCameraZoomIn();

	/* zoom out the camera */
	void OnCameraZoomOut();

	/* lock the camera on the player */
	void OnCameraLock();

	/* template version for simple input */
	template<int Index>
	void OnUseSkill()
	{
		OnUseSkill(Index);
	}

	/* template version for simple input finished */
	template<int Index>
	void OnUseSkillFinished()
	{
		OnUseSkillFinished(Index);
	}

	/* template version for simple input upgrade */
	template<int Index>
	void OnUpgradeSkill()
	{
		OnUpgradeSkill(Index);
	}

	void OnUpgradeSkill(int32 index);

	/* function for accepting mod use */
	template<int Index>
	void OnUseMod()
	{
		OnUseMod(Index);
	}

	/* function for accepting mod use stop */
	template<int Index>
	void OnUseModFinished()
	{
		OnUseModFinished(Index);
	}

	/* player wants to use a mod*/
	void OnUseMod(int32 index);

	/* player stopped pressing mod button */
	void OnUseModFinished(int32 index);

	/* [CLIENT] calculates a world position thats underneath the player's cursor location and sends it to the server */
	void CalculateDirectedMove();

	/* [CLIENT] player wants to clear current commands */
	void ClearCommands();

	/* [CLIENT] toggles the timer that sends movement and attack commands */
	void ToggleMovementSystem(bool bEnabled);

	virtual void Tick(float DeltaSeconds) override;

	/* on self (player character, not spectator) lock */
	void OnSelfCameraLock();

	/* lock the camera on any specified actor */
	void LockCamera(AActor* focusedActor);
	void UnlockCamera();

	/* unlock the camera from any current target */
	void OnUnlockCamera();

	/* server set location, useful for things with the camera */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLocation(FVector newLocation, AActor* attachActor = nullptr);

	/* gets the rotation of the camera with only the yaw value */
	FRotator GetIsolatedCameraYaw();

	/* move the camera in a direction */
	void MoveCamera(FVector worldDirection);
	FVector RightCameraMovement(float direction);
	FVector ForwardCameraMovement(float direction);

public:

	/* get the rts camera */
	UCameraComponent* GetRTSCamera() const
	{
		return rtsCamera;
	}
};