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

	if (!IsValid(Canvas))
		return;

	float ts = GetWorld()->TimeSeconds;

	for (int32 index = 0; index < floatingDamage.Num(); index++)
	{
		float deltaSeconds = ts - floatingDamage[index].originTime;
		if (deltaSeconds < 3.f)
		{
			FVector screenPos = Project(floatingDamage[index].worldPosition);
			//floatingDamage[index].posY = FMath::FInterpConstantTo(floatingDamage[index].posY, floatingDamage[index].posY - 35, GetWorld()->GetDeltaSeconds(), 17.f);

			FString dText = FString::FromInt(floatingDamage[index].amount);
			FColor fontColor = FColor::White;

			//DrawText(dText, fontColor, screenPos.X, screenPos.Y - (deltaSeconds/3.f * 90.f), uiFont);

			if (floatingDamage[index].damageType == UPhysicalDamage::StaticClass())
				Canvas->SetDrawColor(FColor(232.f, 125.f, 143.f));
			else if (floatingDamage[index].damageType == USpecialDamage::StaticClass())
				Canvas->SetDrawColor(FColor(93.f, 123.f, 186.f));
			else
				Canvas->SetDrawColor(fontColor);

			Canvas->DrawText(uiFont, dText, screenPos.X, screenPos.Y - (deltaSeconds / 3.f * 90.f));
		}
		else
			floatingDamage.RemoveAt(index);
	}
}