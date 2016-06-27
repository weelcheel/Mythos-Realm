#include "Realm.h"
#include "PlayerCharacter.h"
#include "RealmGameMode.h"
#include "UnrealNetwork.h"
#include "RealmPlayerController.h"
#include "RealmPlayerState.h"
#include "PlayerHUD.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	credits = 0;
	ambientCreditAmount = 4;

	GetCapsuleComponent()->SetCapsuleRadius(15.f);
}

void APlayerCharacter::BeginPlay()
{
	skillManager = GetWorld()->SpawnActor<ASkillManager>(GetActorLocation(), GetActorRotation());
	skillManager->SetOwner(this);
	shieldManager = GetWorld()->SpawnActor<AShieldManager>();
	shieldManager->SetOwner(this);

	for (int32 i = 0; i < skillClasses.Num(); i++)
	{
		ASkill* newSkill = GetWorld()->SpawnActor<ASkill>(skillClasses[i], GetActorLocation(), GetActorRotation());
		if (newSkill)
		{
			newSkill->InitializeSkill(this);
			skillManager->AddSkill(newSkill);
		}
	}

	Super::BeginPlay();
}

/*void APlayerCharacter::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC) || !IsValid(shieldManager))
		return;

	if (bHidden)
		return;

	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);

	ARealmPlayerController* pc = Cast<ARealmPlayerController>(PC);
	if (IsValid(pc) && pc->GetPlayerCharacter() == this)
		Canvas->SetDrawColor(FColor::Yellow);

	FString playerName = "Player";

	if (IsValid(PlayerState))
		playerName = PlayerState->PlayerName;
	else
		return;

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		//health bar
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y - 6.f), FVector2D(125.f, 18.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y - 6.f), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 125.f, 18.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
		
		//flare bar
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y + 12.f), FVector2D(125.f, 6.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y + 12.f), FVector2D((GetFlare() / GetCurrentValueForStat(EStat::ES_Flare)) * 125.f, 6.f), FVector2D::ZeroVector, FVector2D::UnitVector, FColor::Blue);

		//shield bar
		Canvas->K2_DrawTexture(nullptr, FVector2D(((screenPos.X - (125.f / 2.f)) + ((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 125.f)) - ((shieldManager->GetTotalShieldAmount() / GetCurrentValueForStat(EStat::ES_HP)) * 125.f), screenPos.Y - 6.f), FVector2D((shieldManager->GetTotalShieldAmount() / GetCurrentValueForStat(EStat::ES_HP)) * 125.f, 18.f), FVector2D::ZeroVector, FVector2D::UnitVector, FColor::White);

		Canvas->K2_DrawText(UEngine::GetLargeFont(), playerName, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y - 26.f));
	}
}*/

void APlayerCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage, FDamageRecap& damageDesc)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage, damageDesc);

	if (Role == ROLE_Authority)
	{
		ARealmPlayerController* pc = nullptr;
		AGameCharacter* gc = Cast<AGameCharacter>(InstigatingPawn);
		if (IsValid(gc))
		{
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerDied(IsValid(playerController) ? playerController : GetController(), IsValid(pc) ? pc : gc->GetPlayerController(), gc);
		}
		else
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerDied(IsValid(playerController) ? playerController : GetController(), nullptr, nullptr);
	}

	GetWorldTimerManager().ClearTimer(liftHitsClearTimer);

	if (IsValid(playerController))
	{
		ASpectatorCharacter* sc = Cast<ASpectatorCharacter>(playerController->GetPawn());
		if (IsValid(sc) && IsValid(sc->GetRTSCamera()))
			sc->GetRTSCamera()->PostProcessSettings.ColorSaturation = FVector::ZeroVector;
	}

	if (IsValid(playerController) && IsValid(playerController->GetHUD()))
	{
		APlayerHUD* ph = Cast<APlayerHUD>(playerController->GetHUD());
		if (IsValid(ph))
			ph->PlayerCharacterDied();
	}
}

void APlayerCharacter::StartRespawnTimers_Implementation(float respawnTime)
{
	GetWorldTimerManager().SetTimer(respawnTimer, this, &APlayerCharacter::Respawn, respawnTime);
	GetWorldTimerManager().SetTimer(liftHitsClearTimer, this, &APlayerCharacter::ClearLifeHits, respawnTime, false);
}

void APlayerCharacter::Respawn()
{
	if (Role == ROLE_Authority)
	{
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->RestartPlayer(GetController());

		if (IsValid(playerController))
			PlayerState = playerController->PlayerState;

		AActor* start = GetWorld()->GetAuthGameMode<ARealmGameMode>()->FindPlayerStart(IsValid(playerController) ? playerController : GetController());
		if (start)
			SetActorLocation(start->GetActorLocation());
		else
			SetActorLocation(IsValid(playerController) ? playerController->StartSpot->GetActorLocation() : GetController()->StartSpot->GetActorLocation());

		statsManager->SetMaxFlare();
		statsManager->SetMaxHealth();

		bReplicateMovement = true;
		lifeHits.Empty();
	}

	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	bIsDying = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	//StopRagdollPhysics();
	SetActorEnableCollision(true);

	ClearLifeHits();

	OnCharacterSpawned();

	if (IsValid(playerController))
	{
		ASpectatorCharacter* sc = Cast<ASpectatorCharacter>(playerController->GetPawn());
		if (IsValid(sc) && IsValid(sc->GetRTSCamera()))
			sc->GetRTSCamera()->PostProcessSettings.ColorSaturation = FVector(1.f, 1.f, 1.f);
	}

	if (IsValid(playerController) && IsValid(playerController->GetHUD()))
	{
		APlayerHUD* ph = Cast<APlayerHUD>(playerController->GetHUD());
		if (IsValid(ph))
			ph->PlayerCharacterRespawned();
	}
}

void APlayerCharacter::ChangeCredits(int32 deltaAmount, const FVector& worldLoc)
{
	if (FMath::Abs(deltaAmount) > CREDIT_CHANGE_MAX)
		return;

	credits += deltaAmount;

	if (IsValid(GetPlayerController()))
	{
		ARealmPlayerState* ps = Cast<ARealmPlayerState>(GetPlayerController()->PlayerState);
		if (IsValid(ps))
			ps->playerTotalIncome += deltaAmount;

		ARealmPlayerController* pc = Cast<ARealmPlayerController>(GetPlayerController());
		if (IsValid(pc))
		{
			if (deltaAmount > 0)
				pc->ClientShowCreditGain(worldLoc, deltaAmount);
		}
	}
}

void APlayerCharacter::ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled, FRealmDamage& realmDamage, FDamageRecap& damageDesc)
{
	Super::ReplicateHit(damage, damageEvent, instigatingPawn, damageCauser, bKilled, realmDamage, damageDesc);

	if (Role == ROLE_Authority && GetPlayerController() != nullptr)
	{
		GetPlayerController()->ServerStopBaseTeleport();

		TArray<FHitResult> hits;
		FVector start = GetActorLocation();
		FVector end = start;
		end.Z += 5.f;

		GetWorld()->SweepMultiByChannel(hits, start, end, GetActorRotation().Quaternion(), ECC_Pawn, FCollisionShape::MakeSphere(420.f));

		for (FHitResult hit : hits)
		{
			AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
			AGameCharacter* enemy = Cast<AGameCharacter>(instigatingPawn);
			if (IsValid(gc) && gc->IsAlive() && gc->GetTeamIndex() == GetTeamIndex() && IsValid(enemy))
				gc->ReceiveCallForHelp(this, enemy);
		}
	}

	lifeHits.Add(lastTakeHitInfo);

	GetWorldTimerManager().SetTimer(liftHitsClearTimer, this, &APlayerCharacter::ClearLifeHits, 8.5f, false);
}

void APlayerCharacter::StartAmbientCreditIncome(int32 amount)
{
	ambientCreditAmount = amount;

	GetWorldTimerManager().SetTimer(ambientCreditTimer, this, &APlayerCharacter::OnAmbientCreditIncome, 0.25f, true);
}

void APlayerCharacter::OnAmbientCreditIncome()
{
	credits += ambientCreditAmount / 4.f;
}

void APlayerCharacter::ClearLifeHits()
{
	lifeHits.Empty();
}

void APlayerCharacter::StartBaseTeleport_Implementation()
{
	if (bIsDying)
		return;

	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(baseTeleportTimer, this, &APlayerCharacter::PerformBaseTeleport, 7.f, false);
		GetCharacterMovement()->StopMovementImmediately();
		ApplyCharacterAction("Base Teleport", 7.f, true);
	}
	else
		GetWorldTimerManager().SetTimer(baseTeleportTimer, 7.f, false);
}

void APlayerCharacter::StopBaseTeleport_Implementation()
{
	if (GetWorldTimerManager().GetTimerRemaining(baseTeleportTimer) > 0.f)
	{
		GetWorldTimerManager().ClearTimer(baseTeleportTimer);
		GetWorldTimerManager().ClearTimer(actionTimer);
	}
}

void APlayerCharacter::PerformBaseTeleport_Implementation()
{
	if (bIsDying)
		return;

	if (Role == ROLE_Authority)
	{
		AActor* start = GetWorld()->GetAuthGameMode<ARealmGameMode>()->FindPlayerStart(playerController);
		if (start)
			SetActorLocation(start->GetActorLocation());
		else
			SetActorLocation(playerController->StartSpot->GetActorLocation());
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, credits);
	DOREPLIFETIME(APlayerCharacter, lifeHits);
}