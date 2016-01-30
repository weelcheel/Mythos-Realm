#include "Realm.h"
#include "RealmEnabler.h"
#include "RealmPlayerController.h"
#include "RealmGameMode.h"
#include "PlayerCharacter.h"
#include "Effect.h"

#define LOCTEXT_NAMESPACE "Realm" 

ARealmEnabler::ARealmEnabler(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void ARealmEnabler::PlayerOpenedStore(ARealmPlayerController* pc)
{
	
}

void ARealmEnabler::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		FTimerHandle h;
		GetWorldTimerManager().SetTimer(h, this, &ARealmEnabler::OnTargetsUpdate, 0.25f, true);
	}
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
			if (IsValid(pc->GetStatsManager()) && protectedPlayers.Contains(pc) && protectedPlayers.Remove(pc) > 0)
				EnablerEffectFinished(pc);
		}
		else if (!protectedPlayers.Contains(pc) && protectedPlayers.AddUnique(pc) >= 0)
		{
			enablerAuraEffect = GetWorld()->SpawnActor<AEffect>(AEffect::StaticClass());

			//effect descriptions
			enablerAuraEffect->uiName = LOCTEXT("enablereffect", "Enabler Protection Aura");
			enablerAuraEffect->description = LOCTEXT("enablereffectdesc", "This unit is under protection from their Enabler and has increased Health and Flare regeneration.");
			enablerAuraEffect->keyName = "enablerprotection";
			enablerAuraEffect->bCanBeInflictedMultipleTimes = false;

			//effect stat changes
			enablerAuraEffect->stats.AddUnique(EStat::ES_HPRegen);
			enablerAuraEffect->stats.AddUnique(EStat::ES_FlareRegen);
			enablerAuraEffect->amounts.Add(50.f);
			enablerAuraEffect->amounts.Add(50.f);

			pc->GetStatsManager()->AddCreatedEffect(enablerAuraEffect);
		}
	}
}

void ARealmEnabler::EnablerEffectFinished(AGameCharacter* gc)
{
	if (IsValid(gc))
	{
		AStatsManager* sm = gc->GetStatsManager();
		if (IsValid(sm))
		{
			AEffect* effect = sm->GetEffect("enablerprotection");
			if (IsValid(effect))
				sm->EffectFinished(effect->keyName);
		}
	}
}

void ARealmEnabler::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);

	if (Role == ROLE_Authority && GetWorld()->GetAuthGameMode<ARealmGameMode>())
	{
		if (GetTeamIndex() == 0)
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->EnablerDestroyed(this, 1);
		else if (GetTeamIndex() == 1)
			GetWorld()->GetAuthGameMode<ARealmGameMode>()->EnablerDestroyed(this, 0);
	}
}

void ARealmEnabler::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
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

		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D(150.f, 20.f), FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::Black);
		Canvas->K2_DrawTexture(nullptr, FVector2D(screenPos.X - (150.f / 2.f), screenPos.Y), FVector2D((GetHealth() / GetCurrentValueForStat(EStat::ES_HP)) * 150.f, 20.f), FVector2D::ZeroVector, FVector2D::UnitVector, Canvas->DrawColor);
	}
}

#undef LOCTEXT_NAMESPACE 