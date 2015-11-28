#include "Realm.h"
#include "PlayerHUD.h"
#include "GameCharacter.h"

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

void APlayerHUD::NewDamageEvent(FTakeHitInfo hitInfo, FVector worldPosition)
{
	FUIDamage dmg;

	dmg.amount = hitInfo.ActualDamage;
	dmg.originTime = GetWorld()->TimeSeconds;
	dmg.worldPosition = worldPosition;

	FVector screenPos = Project(dmg.worldPosition);
	dmg.posY = screenPos.Y;

	floatingDamage.Add(dmg);
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
			FLinearColor fontColor = FLinearColor::White;

			DrawText(dText, fontColor, screenPos.X, screenPos.Y - (deltaSeconds/3.f * 90.f), uiFont);
		}
		else
			floatingDamage.RemoveAt(index);
	}
}