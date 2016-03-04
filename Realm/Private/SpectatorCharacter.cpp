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

	RootComponent = capsule;
	capsule->SetCollisionResponseToAllChannels(ECR_Ignore);

	springArm->AttachParent = RootComponent;

	springArm->TargetArmLength = 0.f;
	springArm->TargetOffset.X = -1000 * FMath::Cos(45.f);
	springArm->TargetOffset.Y = 1000 * FMath::Cos(45.f);
	springArm->TargetOffset.Z = 1420 * FMath::Cos(45.f);
	springArm->bInheritPitch = false;
	springArm->bInheritRoll = false;
	springArm->bInheritYaw = false;

	rtsCamera->AttachTo(springArm, TEXT(""), EAttachLocation::SnapToTarget);
	rtsCamera->SetWorldRotation(FRotator(-45.f, -45.f, 0.f));
	rtsCamera->PostProcessSettings.bOverride_ColorSaturation = true;

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
}

void ASpectatorCharacter::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (bMoveCommand && IsValid(this))
	{
		CalculateDirectedMove();
	}

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
	}

	if (Role < ROLE_Authority)
	{
		if (camDirection != FVector::ZeroVector)
			MoveCamera(camDirection * camSpeed);
		//else
			//MoveCamera(GetCharacterMovement()->Velocity.IsNearlyZero(40.f) ? FVector::ZeroVector : GetCharacterMovement()->Velocity * -1);
	}

	if (Role == ROLE_Authority)
	{ 
		if (IsValid(cameraLockTarget))
			SetActorLocation(cameraLockTarget->GetActorLocation());
	}
}

void ASpectatorCharacter::OnDirectedMoveStart()
{
	bMoveCommand = true;
}

void ASpectatorCharacter::OnDirectedMoveStop()
{
	bMoveCommand = false;
}

void ASpectatorCharacter::CalculateDirectedMove()
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetController());
	FHitResult hit;

	if (pc && pc->SelectUnitUnderMouse(ECC_Visibility, true, hit))
	{
		if (IsValid(hit.GetActor()) && hit.GetActor()->IsA(AGameCharacter::StaticClass()))
		{
			AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
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
		pc->ServerUseSkill(index, hit.Location);
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
			GetWorldTimerManager().SetTimer(movementTimer, this, &ASpectatorCharacter::CalculateDirectedMove, 0.1f, true);
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