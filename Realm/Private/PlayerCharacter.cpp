#include "Realm.h"
#include "PlayerCharacter.h"
#include "RealmGameMode.h"
#include "UnrealNetwork.h"
#include "RealmPlayerController.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	credits = 500;
}

void APlayerCharacter::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC->GetCharacter()))
		return;

	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);

	ARealmPlayerController* pc = Cast<ARealmPlayerController>(PC);
	if (IsValid(pc) && pc->GetPlayerCharacter() == this)
		Canvas->SetDrawColor(FColor::Yellow);

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (115.f / 2.f), screenPos.Y), FVector2D(115.f, 15.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (115.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 115.f, 15.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
	}
}

void APlayerCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);

	AGameCharacter* gc = Cast<AGameCharacter>(InstigatingPawn);
	if (Role == ROLE_Authority && IsValid(gc))
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerDied(playerController, gc->GetPlayerController());

	float respawnTime = 10 + GetWorld()->TimeSeconds / 10.f;
	GetWorldTimerManager().SetTimer(respawnTimer, this, &APlayerCharacter::Respawn, respawnTime);
}

void APlayerCharacter::Respawn()
{
	if (Role == ROLE_Authority)
	{
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->RestartPlayer(GetController());

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

	lifeHits.Empty();
}

void APlayerCharacter::ChangeCredits(int32 deltaAmount)
{
	if (FMath::Abs(deltaAmount) > CREDIT_CHANGE_MAX)
		return;

	credits += deltaAmount;
}

void APlayerCharacter::ReplicateHit(float damage, struct FDamageEvent const& damageEvent, class APawn* instigatingPawn, class AActor* damageCauser, bool bKilled)
{
	Super::ReplicateHit(damage, damageEvent, instigatingPawn, damageCauser, bKilled);

	lifeHits.Add(lastTakeHitInfo);
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

void APlayerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, credits);
}