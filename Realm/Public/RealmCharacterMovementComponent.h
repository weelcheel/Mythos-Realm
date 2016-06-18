#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "RealmCharacterMovementComponent.generated.h"

UCLASS()
class URealmCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

protected:

	/* is the character dashing */
	bool bCharacterWantsDash, bCharacterDashing;
	FVector targetDashLocation;

	//Init
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/* timer for keeping track of how long this character cannot move for */
	FTimerHandle ignoreMovementTimer;

	/* current fly speed scale for dashes */
	float flySpeedScale = 1.f;

public:

	/* override launch for our use */
	void DashLaunch(FVector const& endLocation, float spdScale = 1.f);
	virtual bool HandlePendingLaunch() override;

	UFUNCTION(BlueprintCallable, Category = Dash)
	void EndDash();

	/* ignore movement commands for a certain period of time */
	void IgnoreMovement();
	void IgnoreMovementForDuration(float duration);

	/* re enabale movement */
	void ClearMovementIgnorance();
};