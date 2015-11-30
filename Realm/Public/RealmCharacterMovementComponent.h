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

public:

	/* override launch for our use */
	void DashLaunch(FVector const& endLocation);
	virtual bool HandlePendingLaunch() override;

	UFUNCTION(BlueprintCallable, Category = Dash)
	void EndDash();
};