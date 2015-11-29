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
		{
			bCharacterDashing = false;
			StopMovementImmediately();
			SetMovementMode(MOVE_Walking);

			AGameCharacter* gc = Cast<AGameCharacter>(CharacterOwner);
			if (IsValid(gc))
				gc->CharacterDashFinished();
		}
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