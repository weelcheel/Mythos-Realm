#include "Realm.h"
#include "PlayerCharacter.h"
#include "RealmGameMode.h"
#include "UnrealNetwork.h"
#include "RealmPlayerController.h"
#include "RealmPlayerState.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	credits = 0;
	ambientCreditAmount = 4;
}

void APlayerCharacter::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC->GetCharacter()))
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

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y), FVector2D(125.f, 18.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 125.f, 18.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
		Canvas->K2_DrawText(UEngine::GetLargeFont(), playerName, FVector2D(screenPos.X - (125.f / 2.f), screenPos.Y - 20.f));
	}
}

void APlayerCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage);

	if (Role == ROLE_Authority)
	{
		ARealmPlayerController* pc = nullptr;
		AGameCharacter* gc = Cast<AGameCharacter>(InstigatingPawn);
		if (IsValid(gc))
		{
			for (int32 i = 0; i < lifeHits.Num(); i++)
			{
				if (IsValid(lifeHits[i].PawnInstigator) && IsValid(lifeHits[i].PawnInstigator->GetPlayerController()))
					pc = lifeHits[i].PawnInstigator->GetPlayerController();
			}
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerDied(playerController, IsValid(pc) ? pc : gc->GetPlayerController(), gc);
		}
		else
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerDied(playerController, nullptr, nullptr);

		GetWorldTimerManager().ClearTimer(liftHitsClearTimer);
	}

	float respawnTime = GetWorld()->TimeSeconds / 10.f;
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

		AActor* start = GetWorld()->GetAuthGameMode<ARealmGameMode>()->FindPlayerStart(playerController);
		if (start)
			SetActorLocation(start->GetActorLocation());
		else
			SetActorLocation(playerController->StartSpot->GetActorLocation());

		statsManager->SetMaxFlare();
		statsManager->SetMaxHealth();

		bReplicateMovement = true;
		lifeHits.Empty();
	}

	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	bIsDying = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	//StopRagdollPhysics();
	SetActorEnableCollision(true);

	ClearLifeHits();

	OnCharacterSpawned();
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

void APlayerCharacter::ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled, FRealmDamage& realmDamage)
{
	Super::ReplicateHit(damage, damageEvent, instigatingPawn, damageCauser, bKilled, realmDamage);

	lifeHits.Add(lastTakeHitInfo);
	GetWorldTimerManager().SetTimer(liftHitsClearTimer, this, &APlayerCharacter::ClearLifeHits, 7.5f, false);
}

void APlayerCharacter::StartAmbientCreditIncome(int32 amount)
{
	ambientCreditAmount = amount;

	GetWorldTimerManager().SetTimer(ambientCreditTimer, this, &APlayerCharacter::OnAmbientCreditIncome, 0.25f, true);
}

void APlayerCharacter::OnAmbientCreditIncome()
{
	ChangeCredits(ambientCreditAmount / 4.f);
}

void APlayerCharacter::ClearLifeHits()
{
	lifeHits.Empty();
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, credits);
	DOREPLIFETIME(APlayerCharacter, lifeHits);
}