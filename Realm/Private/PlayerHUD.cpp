#include "Realm.h"
#include "PlayerHUD.h"
#include "GameCharacter.h"
#include "DamageTypes.h"

APlayerHUD::APlayerHUD(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	//Font'/Game/Realm/UI/UIFont.UIFont'
	static ConstructorHelpers::FObjectFinder<UFont> fontClass(TEXT("/Game/Realm/UI/UIFont"));
	if (fontClass.Succeeded())
	{
		uiFont = fontClass.Object;
	}

	bShowOverlays = true;
}

void APlayerHUD::NewDamageEvent(FTakeHitInfo hitInfo, FVector worldPosition, FRealmDamage& realmdmg)
{
	FUIDamage dmg;

	dmg.amount = hitInfo.ActualDamage;
	dmg.originTime = GetWorld()->TimeSeconds;
	dmg.worldPosition = worldPosition;
	dmg.damageType = hitInfo.DamageTypeClass;
	dmg.realmDamage = realmdmg;

	FVector2D screenPos;
	UGameplayStatics::ProjectWorldToScreen(PlayerOwner, dmg.worldPosition, screenPos);

	//floatingDamage.Add(dmg);
	ShowDamageText(dmg);
}

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AGameCharacter> objItr(GetWorld()); objItr; ++objItr)
	{
		AddPostRenderedActor((*objItr));
	}
}

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	//DrawActorOverlays(FVector::ZeroVector, FRotator::ZeroRotator);
}