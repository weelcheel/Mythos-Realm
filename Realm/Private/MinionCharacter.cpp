#include "Realm.h"
#include "MinionCharacter.h"
#include "RealmMoveController.h"
#include "RealmLaneMinionAI.h"
#include "PlayerCharacter.h"
#include "RealmPlayerState.h"
#include "RealmForestMinionAI.h"
#include "RealmPlayerController.h"

AMinionCharacter::AMinionCharacter(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	playerReward = FMath::RandRange(28.f, 34.f);
	AIControllerClass = nullptr;
	bReplicateMovement = true;
}

void AMinionCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, FRealmDamage& realmDamage)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, realmDamage);

	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (Role == ROLE_Authority)
	{
		if (IsValid(GetController()))
		{
			GetWorldTimerManager().ClearAllTimersForObject(GetController());
			GetController()->SetLifeSpan(2.6f);
		}

		FTimerHandle timer;
		GetWorldTimerManager().SetTimer(timer, this, &AMinionCharacter::RealmDestroy, 2.6f, false);
	}

	APlayerCharacter* pc = Cast<APlayerCharacter>(InstigatingPawn);
	if (Role == ROLE_Authority && IsValid(pc))
	{
		pc->ChangeCredits(playerReward);
		if (IsValid(pc->GetPlayerController()))
		{
			ARealmPlayerState* ps = Cast<ARealmPlayerState>(pc->GetPlayerController()->PlayerState);
			if (IsValid(ps))
				ps->playerCreepScore++;
		}
	}
}

void AMinionCharacter::RealmDestroy()
{
	Destroy();
}

float AMinionCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	ARealmForestMinionAI* ai = Cast<ARealmForestMinionAI>(GetController());
	if (IsValid(ai))
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>(EventInstigator);
		if (IsValid(pc) && IsValid(pc->GetPlayerCharacter()))
			ai->CharacterTookDamage(pc->GetPlayerCharacter());
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AMinionCharacter::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC->GetCharacter()))
		return;

	if (bHidden)
		return;

	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (75.f / 2.f), screenPos.Y), FVector2D(75.f, 5.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (75.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 75.f, 5.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
	}
}

void AMinionCharacter::ReceiveCallForHelp(AGameCharacter* distressedUnit, AGameCharacter* enemyTarget)
{
	ARealmLaneMinionAI* aipc = Cast<ARealmLaneMinionAI>(GetController());

	if (IsValid(aipc))
		aipc->ReceiveCallForHelp(distressedUnit, enemyTarget);
}