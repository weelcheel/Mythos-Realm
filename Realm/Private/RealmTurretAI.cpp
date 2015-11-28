#include "Realm.h"
#include "RealmTurretAI.h"
#include "RealmTurret.h"

ARealmTurretAI::ARealmTurretAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	pawnSensor = objectInitializer.CreateDefaultSubobject<UPawnSensingComponent>(this, TEXT("turretSensor"));
	pawnSensor->OnSeePawn.AddDynamic(this, &ARealmTurretAI::OnSeePawn);
	pawnSensor->SightRadius = 710.f;
	pawnSensor->bOnlySensePlayers = false;
	pawnSensor->SensingInterval = 0.25f;
	pawnSensor->SetPeripheralVisionAngle(180.f);
}

void ARealmTurretAI::OnSeePawn(class APawn* pawn)
{
	ATurret* turret = Cast<ATurret>(GetPawn());
	if (IsValid(turret))
		turret->OnSeePawn(pawn);
}