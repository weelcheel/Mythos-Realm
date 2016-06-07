#include "Realm.h"
#include "SpectatorCharacter.h"
#include "RealmPlayerController.h"
#include "PlayerCharacter.h"
#include "GameCharacter.h"

ASpectatorCharacter::ASpectatorCharacter(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	springArm = objectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	rtsCamera = objectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("RTS Camera"));
	capsule = objectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule"));
	hoverLight = objectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("Hover Light"));

	RootComponent = capsule;
	capsule->SetCollisionResponseToAllChannels(ECR_Ignore);

	springArm->AttachParent = RootComponent;

	springArm->TargetArmLength = 0.f;

	//springArm->TargetOffset.X = -500.f;
	//springArm->TargetOffset.Y = 500.f;
	//springArm->TargetOffset.Z = 1000.f;
	cameraOffset = FVector(-500.f, 500.f, 1000.f);

	springArm->bInheritPitch = false;
	springArm->bInheritRoll = false;
	springArm->bInheritYaw = false;

	rtsCamera->AttachTo(springArm, TEXT(""), EAttachLocation::SnapToTarget);
	rtsCamera->SetWorldRotation(FRotator(-60.f, -45.f, 0.f));
	rtsCamera->PostProcessSettings.bOverride_ColorSaturation = true;
	/*rtsCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	rtsCamera->OrthoWidth = 2150.f;
	rtsCamera->OrthoFarClipPlane = 5000.f;
	rtsCamera->OrthoNearClipPlane = 0.f;*/
	zoomFactor = 1.f;
	maxZoomFactorDelta = 0.33f;

	hoverLight->SetAttenuationRadius(50.f);
	hoverLight->SetSourceRadius(50.f);
	hoverLight->Intensity = 3.f;
	hoverLight->SetVisibility(false);

	//GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	camSpeed = 35.f;

	bReplicates = true;
}

void ASpectatorCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASpectatorCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAction("MoveCommand", IE_Pressed, this, &ASpectatorCharacter::OnDirectedMoveStart);
	InputComponent->BindAction("MoveCommand", IE_Released, this, &ASpectatorCharacter::OnDirectedMoveStop);

	//skills
	InputComponent->BindAction("Skill1", IE_Pressed, this, &ASpectatorCharacter::OnUseSkill<0>);
	InputComponent->BindAction("Skill1", IE_Released, this, &ASpectatorCharacter::OnUseSkillFinished<0>);
	InputComponent->BindAction("Skill2", IE_Pressed, this, &ASpectatorCharacter::OnUseSkill<1>);
	InputComponent->BindAction("Skill2", IE_Released, this, &ASpectatorCharacter::OnUseSkillFinished<1>);
	InputComponent->BindAction("Skill3", IE_Pressed, this, &ASpectatorCharacter::OnUseSkill<2>);
	InputComponent->BindAction("Skill3", IE_Released, this, &ASpectatorCharacter::OnUseSkillFinished<2>);
	InputComponent->BindAction("Skill4", IE_Pressed, this, &ASpectatorCharacter::OnUseSkill<3>);
	InputComponent->BindAction("Skill4", IE_Released, this, &ASpectatorCharacter::OnUseSkillFinished<3>);

	//upgrade skills
	InputComponent->BindAction("UpgradeSkill1", IE_Pressed, this, &ASpectatorCharacter::OnUpgradeSkill<0>);
	InputComponent->BindAction("UpgradeSkill2", IE_Pressed, this, &ASpectatorCharacter::OnUpgradeSkill<1>);
	InputComponent->BindAction("UpgradeSkill3", IE_Pressed, this, &ASpectatorCharacter::OnUpgradeSkill<2>);
	InputComponent->BindAction("UpgradeSkill4", IE_Pressed, this, &ASpectatorCharacter::OnUpgradeSkill<3>);

	InputComponent->BindAction("SelfCameraLock", IE_Pressed, this, &ASpectatorCharacter::OnSelfCameraLock);
	InputComponent->BindAction("SelfCameraLock", IE_Released, this, &ASpectatorCharacter::OnUnlockCamera);

	//mods
	InputComponent->BindAction("Mod1", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<0>);
	InputComponent->BindAction("Mod1", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<0>);
	InputComponent->BindAction("Mod2", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<1>);
	InputComponent->BindAction("Mod2", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<1>);
	InputComponent->BindAction("Mod3", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<2>);
	InputComponent->BindAction("Mod3", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<2>);
	InputComponent->BindAction("Mod4", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<3>);
	InputComponent->BindAction("Mod4", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<3>);
	InputComponent->BindAction("Mod5", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<4>);
	InputComponent->BindAction("Mod5", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<4>);
	InputComponent->BindAction("Mod6", IE_Pressed, this, &ASpectatorCharacter::OnUseMod<5>);
	InputComponent->BindAction("Mod6", IE_Released, this, &ASpectatorCharacter::OnUseModFinished<5>);

	//chat
	InputComponent->BindAction("PlayerToggleChat", IE_Pressed, this, &ASpectatorCharacter::OnToggleChat);

	//camera
	InputComponent->BindAction("CameraZoomIn", IE_Pressed, this, &ASpectatorCharacter::OnCameraZoomIn);
	InputComponent->BindAction("CameraZoomOut", IE_Pressed, this, &ASpectatorCharacter::OnCameraZoomOut);
}

void ASpectatorCharacter::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	FVector2D mousePosition;
	FVector2D viewportSize;

	UGameViewportClient* gameViewport = GEngine->GameViewport;

	if (!gameViewport)
		return;

	gameViewport->GetViewportSize(viewportSize);

	FVector camDirection = FVector::ZeroVector;

	if (gameViewport->IsFocused(gameViewport->Viewport) && gameViewport->GetMousePosition(mousePosition))
	{
		//Check if the mouse is at the left or right edge of the screen and move accordingly
		if (mousePosition.X / viewportSize.X <= 0.04f)
		{
			//MoveCameraRight(-1.0f * deltaSeconds);
			camDirection += RightCameraMovement(-1.0f * deltaSeconds);
		}
		else if (mousePosition.X / viewportSize.X >= 0.96f)
		{
			camDirection += RightCameraMovement(1.0f * deltaSeconds);
		}

		//Check if the mouse is at the top or bottom edge of the screen and move accordingly
		if (mousePosition.Y / viewportSize.Y <= 0.04f)
		{
			camDirection += ForwardCameraMovement(1.0f * deltaSeconds);
		}
		else if (mousePosition.Y / viewportSize.Y >= 0.96f)
		{
			camDirection += ForwardCameraMovement(-1.0f * deltaSeconds);
		}

		//check for hovering targets to display targeting aura's
		ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
		if (IsValid(pc))
		{
			FHitResult hit;
			if (pc->SelectUnitUnderMouse(ECC_Visibility, true, hit) && IsValid(Cast<AGameCharacter>(hit.GetActor())))
				SetHoverTarget(Cast<AGameCharacter>(hit.GetActor()));
			else
				RemoveHoverTarget();
		}
	}

	if (Role < ROLE_Authority)
	{
		if (camDirection != FVector::ZeroVector)
			MoveCamera(camDirection * camSpeed);
		//else
			//MoveCamera(GetCharacterMovement()->Velocity.IsNearlyZero(40.f) ? FVector::ZeroVector : GetCharacterMovement()->Velocity * -1);
	}

	springArm->TargetOffset = FMath::VInterpTo(springArm->TargetOffset, cameraOffset * zoomFactor, deltaSeconds, 4.f);
}

void ASpectatorCharacter::SetHoverTarget(AGameCharacter* newTarget)
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	if (!IsValid(pc) || !IsValid(pc->GetPlayerCharacter()))
		return;

	if (IsValid(newTarget))
		hoverTarget = newTarget;
	else
		return;

	if (hoverTarget->GetTeamIndex() == pc->GetPlayerCharacter()->GetTeamIndex())
		hoverLight->SetLightColor(FColor::Cyan);
	else
		hoverLight->SetLightColor(FColor::Red);

	hoverLight->AttachTo(hoverTarget->GetRootComponent());
	hoverLight->SetVisibility(true);

	hoverLight->SetAttenuationRadius(hoverTarget->GetSimpleCollisionRadius() * 15.f);
	hoverLight->SetSourceRadius(hoverTarget->GetSimpleCollisionRadius() * 10.f);
	hoverLight->SetRelativeLocation(FVector(0.f, 0.f, -0.85f* hoverTarget->GetSimpleCollisionHalfHeight()));
}

void ASpectatorCharacter::RemoveHoverTarget()
{
	if (hoverTarget != nullptr)
	{
		hoverLight->DetachFromParent();
		hoverLight->SetVisibility(false);

		hoverTarget = nullptr;
	}
}

void ASpectatorCharacter::OnDirectedMoveStart()
{
	ToggleMovementSystem(true);
}

void ASpectatorCharacter::OnDirectedMoveStop()
{
	ToggleMovementSystem(false);
}

void ASpectatorCharacter::CalculateDirectedMove()
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	FHitResult hit;

	if (pc && pc->SelectUnitUnderMouse(ECC_Visibility, true, hit))
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
		if (IsValid(gc) && gc->IsAlive())
		{
			int32 team1 = gc->GetTeamIndex();
			int32 team2 = pc->GetPlayerCharacter()->GetTeamIndex();

			if (gc && team1 != team2)// && !playerController->IsCharacterOnTeam(mc->GetTeam()))
				pc->ServerStartAutoAttack(gc);
			else
				pc->ServerMoveCommand(hit.Location);
		}
		else
			pc->ServerMoveCommand(hit.Location);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Unable to get mouse coordiantes."));
}

void ASpectatorCharacter::OnUseSkill(int32 index)
{
	FHitResult hit;
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());

	if (pc)
	{
		pc->SelectUnitUnderMouse(ECC_Visibility, true, hit);
		pc->ServerUseSkill(index, hit);
	}
}

void ASpectatorCharacter::OnUseSkillFinished(int32 index)
{

}

void ASpectatorCharacter::OnUseMod(int32 index)
{
	FHitResult hit;
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());

	if (IsValid(pc))
	{
		pc->SelectUnitUnderMouse(ECC_Visibility, true, hit);
		pc->ServerUseMod(index, hit);
	}
}

void ASpectatorCharacter::OnUseModFinished(int32 index)
{

}

void ASpectatorCharacter::OnUpgradeSkill(int32 index)
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	if (IsValid(pc))
		pc->ServerOnUpgradeSkill(index);
}

void ASpectatorCharacter::ToggleMovementSystem(bool bEnabled)
{
	if (bEnabled)
	{
		CalculateDirectedMove();

		if (!GetWorldTimerManager().IsTimerActive(movementTimer))
			GetWorldTimerManager().SetTimer(movementTimer, this, &ASpectatorCharacter::CalculateDirectedMove, (1.f / 30.f), true);
	}
	else
	{
		if (GetWorldTimerManager().IsTimerActive(movementTimer))
			GetWorldTimerManager().ClearTimer(movementTimer);
	}
}

FRotator ASpectatorCharacter::GetIsolatedCameraYaw()
{
	return FRotator(0.0f, rtsCamera->ComponentToWorld.Rotator().Yaw, 0.0f);
}

void ASpectatorCharacter::MoveCamera(FVector worldDirection)
{
	//AddMovementInput(worldDirection * camSpeed, 1.f);
	SetActorLocation(GetActorLocation() + worldDirection);
}

FVector ASpectatorCharacter::RightCameraMovement(float direction)
{
	float movementValue = direction * camSpeed;

	FVector deltaMovement = movementValue * (FRotator(0.0f, 90.0f, 0.0f) + GetIsolatedCameraYaw()).Vector();
	return deltaMovement;
}

FVector ASpectatorCharacter::ForwardCameraMovement(float direction)
{
	float movementValue = direction * camSpeed;

	FVector deltaMovement = movementValue * GetIsolatedCameraYaw().Vector();
	return deltaMovement;
}

void ASpectatorCharacter::OnSelfCameraLock()
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	if (!IsValid(pc) || !IsValid(pc->GetPlayerCharacter()))
		return;

	springArm->AttachTo(pc->GetPlayerCharacter()->GetRootComponent());
	cameraLockTarget = pc->GetPlayerCharacter();
	//playerController->ServerLockPlayerCamera(pc->GetPlayerCharacter());
}

void ASpectatorCharacter::OnUnlockCamera()
{
	if (cameraLockTarget)
		SetActorLocation(cameraLockTarget->GetActorLocation());

	springArm->DetachFromParent(false);
	springArm->AttachTo(GetRootComponent());
}

bool ASpectatorCharacter::ServerSetLocation_Validate(FVector newLocation, AActor* attachActor = nullptr)
{
	return true;
}

void ASpectatorCharacter::ServerSetLocation_Implementation(FVector newLocation, AActor* attachActor = nullptr)
{
	//SetActorLocation(newLocation);
	TeleportTo(newLocation, GetActorRotation());

	if (IsValid(attachActor))
		AttachRootComponentTo(attachActor->GetRootComponent());
	else
		DetachRootComponentFromParent(true);
}

void ASpectatorCharacter::OnToggleChat()
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	if (IsValid(pc))
		pc->ClientToggleChat();
}

void ASpectatorCharacter::OnCameraZoomIn()
{
	if (zoomFactor - 0.05f <= maxZoomFactorDelta)
		zoomFactor = maxZoomFactorDelta;
	else
		zoomFactor -= 0.05f;
}

void ASpectatorCharacter::OnCameraZoomOut()
{
	if (zoomFactor + 0.05f >= 1.f)
		zoomFactor = 1.f;
	else
		zoomFactor += 0.05f;
}