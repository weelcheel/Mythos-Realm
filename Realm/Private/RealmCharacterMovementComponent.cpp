#include "Realm.h"
#include "RealmCharacterMovementComponent.h"
#include "GameCharacter.h"

URealmCharacterMovementComponent::URealmCharacterMovementComponent(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void URealmCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void URealmCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCharacterDashing)
	{
		if ((GetCharacterOwner()->GetActorLocation() - targetDashLocation).IsNearlyZero(15.f))
			EndDash();
		else
			GetCharacterOwner()->SetActorLocation(FMath::VInterpConstantTo(GetCharacterOwner()->GetActorLocation(), targetDashLocation, DeltaTime, MaxFlySpeed*2.f));
	}
}

void URealmCharacterMovementComponent::DashLaunch(FVector const& endLocation)
{
	bCharacterWantsDash = true;
	PendingLaunchVelocity = endLocation;
	targetDashLocation = endLocation;
	targetDashLocation.Z = GetCharacterOwner()->GetActorLocation().Z;
}

void URealmCharacterMovementComponent::EndDash()
{
	if (!bCharacterDashing)
		return;

	bCharacterDashing = false;
	StopMovementImmediately();
	SetMovementMode(MOVE_Walking);

	AGameCharacter* gc = Cast<AGameCharacter>(CharacterOwner);
	if (IsValid(gc))
		gc->CharacterDashFinished();
}

bool URealmCharacterMovementComponent::HandlePendingLaunch()
{
	if (!PendingLaunchVelocity.IsZero() && HasValidData())
	{
		Velocity = PendingLaunchVelocity;
		PendingLaunchVelocity = FVector::ZeroVector;
		if (bCharacterWantsDash)
		{
			StopMovementImmediately();
			SetMovementMode(MOVE_Flying);
			bCharacterWantsDash = false;
			bCharacterDashing = true;
		}
		else
			SetMovementMode(MOVE_Falling);

		return true;
	}

	return false;
}

void URealmCharacterMovementComponent::IgnoreMovement()
{
	//end any dashes this character may be performing
	EndDash();

	StopMovementImmediately();
	SetMovementMode(MOVE_None);
}

void URealmCharacterMovementComponent::IgnoreMovementForDuration(float duration)
{
	GetCharacterOwner()->GetWorldTimerManager().SetTimer(ignoreMovementTimer, this, &URealmCharacterMovementComponent::ClearMovementIgnorance, duration);

	IgnoreMovement();
}

void URealmCharacterMovementComponent::ClearMovementIgnorance()
{
	//clear any timers that have been set for clearing this (i.e. if someone gets rid of a stun before its duration is up)
	GetCharacterOwner()->GetWorldTimerManager().ClearTimer(ignoreMovementTimer);

	SetMovementMode(MOVE_Walking);
}

