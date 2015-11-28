#include "Realm.h"
#include "RealmRaiderAI.h"

ARealmRaiderAI::ARealmRaiderAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void ARealmRaiderAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);


}

void ARealmRaiderAI::OnTargetEnterRadius(class APawn* pawn)
{

}