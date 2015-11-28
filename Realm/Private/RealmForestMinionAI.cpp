#include "Realm.h"
#include "RealmForestMinionAI.h"

ARealmForestMinionAI::ARealmForestMinionAI(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void ARealmForestMinionAI::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);


}

void ARealmForestMinionAI::OnTargetEnterRadius(class APawn* pawn)
{

}