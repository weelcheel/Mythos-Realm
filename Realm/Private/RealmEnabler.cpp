#include "Realm.h"
#include "RealmEnabler.h"
#include "RealmPlayerController.h"
#include "RealmGameMode.h"
#include "PlayerCharacter.h"

ARealmEnabler::ARealmEnabler(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	//effect descriptions
	enablerAuraEffect.uiName = "Enabler Protection Aura";
	enablerAuraEffect.description = "This unit is under protection from their Enabler and has increased Health and Flare regeneration.";
	enablerAuraEffect.effectKey = "enablerAura";
	
	//effect stat changes
	enablerAuraEffect.stats.AddUnique(EStat::ES_HPRegen);
	enablerAuraEffect.stats.AddUnique(EStat::ES_FlareRegen);
	enablerAuraEffect.amounts.Add(50.f);
	enablerAuraEffect.amounts.Add(50.f);
}

void ARealmEnabler::PlayerOpenedStore(ARealmPlayerController* pc)
{
	
}

void ARealmEnabler::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle h;
	GetWorldTimerManager().SetTimer(h, this, &ARealmEnabler::OnTargetsUpdate, 0.25f, true);
}

void ARealmEnabler::OnTargetsUpdate()
{
	for (TActorIterator<APlayerCharacter> plyitr(GetWorld()); plyitr; ++plyitr)
	{
		APlayerCharacter* pc = (*plyitr);
		if (!IsValid(pc) || !pc->IsAlive() || pc->GetTeamIndex() != GetTeamIndex())
			continue;

		float distanceSq = (pc->GetActorLocation() - GetActorLocation()).SizeSquared2D();
		if (distanceSq > FMath::Square(auraRange))
		{
			if (IsValid(pc->GetStatsManager()) && protectedPlayers.Remove(pc) > 0)
				pc->GetStatsManager()->EffectFinished(enablerAuraEffect.effectKey);
		}
		else if (protectedPlayers.AddUnique(pc) >= 0)
			pc->AddEffect(enablerAuraEffect.uiName, enablerAuraEffect.description, enablerAuraEffect.effectKey, enablerAuraEffect.stats, enablerAuraEffect.amounts);
	}
}

void ARealmEnabler::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);

	if (Role == ROLE_Authority && GetWorld()->GetAuthGameMode<ARealmGameMode>())
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->EnablerDestroyed(this);
}

void ARealmEnabler::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (!IsValid(PC->GetCharacter()))
		return;

	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);

	if (IsAlive())
	{
		FVector hudPos = GetActorLocation();
		hudPos.Z += 160.f;

		FVector screenPos = Canvas->Project(hudPos);

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D(150.f, 20.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 150.f, 20.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
	}
}