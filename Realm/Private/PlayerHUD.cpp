#include "Realm.h"
#include "PlayerHUD.h"
#include "GameCharacter.h"
#include "DamageTypes.h"
#include "MinimapActor.h"
#include "RealmPlayerController.h"

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
	mapPosition = FVector2D(0.8f, 0.7f);
	mapDimensions = 300.f;
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

	//get overlay actors
	for (TActorIterator<AGameCharacter> objItr(GetWorld()); objItr; ++objItr)
	{
		AddPostRenderedActor((*objItr));
	}

	//then get minimap actors
	for (TActorIterator<AMinimapActor> mapItr(GetWorld()); mapItr; ++mapItr)
		gameMinimap = (*mapItr);
}

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	//DrawActorOverlays(FVector::ZeroVector, FRotator::ZeroRotator);
	DrawMinimap();
}

void APlayerHUD::DrawMinimap()
{
	ARealmPlayerController* pc = Cast<ARealmPlayerController>(PlayerOwner);

	if (!IsValid(pc) || (IsValid(pc) && !IsValid(pc->GetPlayerCharacter())) || !IsValid(gameMinimap))
		return;

	float trueNorth, playerHeading, actualMapRange;
	FVector2D playerPos, displayPlayerPos, startPos;

	const APlayerHUD* defaultHUD = GetDefault<APlayerHUD>();
	if (!IsValid(defaultHUD))
		return;

	mapPosition.X = defaultHUD->mapPosition.X * Canvas->ClipX;
	mapPosition.Y = defaultHUD->mapPosition.Y * Canvas->ClipY;

	actualMapRange = FMath::Max(gameMinimap->mapSizeMax.X - gameMinimap->mapSizeMin.X, gameMinimap->mapSizeMax.Y - gameMinimap->mapSizeMin.Y);

	playerPos = FVector2D::ZeroVector;
	FVector pp = pc->GetPlayerCharacter()->GetActorLocation();
	pp.X = pp.X / actualMapRange;
	pp.Y = pp.Y / actualMapRange;

	playerPos.X = pp.X;
	playerPos.Y = pp.Y;

	trueNorth = gameMinimap->GetRadianHeading();
	playerHeading = GetUnitHeading(pc->GetPlayerCharacter());

	displayPlayerPos.X = playerPos.Size() * FMath::Cos(FMath::Atan2(playerPos.Y, playerPos.X));
	displayPlayerPos.Y = playerPos.Size() * FMath::Sin(FMath::Atan2(playerPos.Y, playerPos.X));
	
	startPos.X = displayPlayerPos.X;
	startPos.Y = displayPlayerPos.Y;

	//draw the back
	Canvas->SetDrawColor(FColor::Black);
	Canvas->K2_DrawMaterial(gameMinimap->mapBackground, mapPosition, FVector2D(mapDimensions / 1920.f * Canvas->ClipX, mapDimensions / 1080.f * Canvas->ClipY), FVector2D::ZeroVector); //draw map back

	//draw this player
	Canvas->SetDrawColor(FColor::Yellow);
	FVector2D pdp = FVector2D(mapPosition.X + mapDimensions * (displayPlayerPos.X), mapPosition.Y + mapDimensions * (displayPlayerPos.Y));
	Canvas->K2_DrawBox(pdp, FVector2D(32.f / 1920.f * Canvas->ClipX, 32.f / 1080.f * Canvas->ClipY));

	//draw other units
	for (TActorIterator<AGameCharacter> unititr(GetWorld()); unititr; ++unititr)
	{
		AGameCharacter* gc = (*unititr);
		if (gc->IsAlive() && !gc->bHidden) //draw visible and alive units
		{
			playerHeading = GetUnitHeading(gc);

			pp = gc->GetActorLocation();
			pp.X = pp.X / actualMapRange;
			pp.Y = pp.Y / actualMapRange;

			displayPlayerPos.X = pp.Size() * FMath::Cos(FMath::Atan2(pp.Y, pp.X));
			displayPlayerPos.Y = pp.Size() * FMath::Sin(FMath::Atan2(pp.Y, pp.X));

			Canvas->SetDrawColor(FColor::Green);
			pdp = FVector2D(mapPosition.X + mapDimensions * (displayPlayerPos.X), mapPosition.Y + mapDimensions * (displayPlayerPos.Y));
			Canvas->K2_DrawBox(pdp, FVector2D(20.f / 1920.f * Canvas->ClipX, 20.f / 1080.f * Canvas->ClipY));
		}
	}
}

float APlayerHUD::GetUnitHeading(AGameCharacter* unit) const
{
	if (!IsValid(unit))
		return -1.f;

	FVector vec;
	FRotator rot;
	float radians;

	//only need a vector with this actor's 
	rot.Yaw = unit->GetActorRotation().Yaw;
	vec = rot.Vector();

	radians = vec.HeadingAngle();
	radians = FMath::UnwindRadians(radians);

	while (radians < 0.f)
		radians += PI * 2.0f;

	return radians;
}