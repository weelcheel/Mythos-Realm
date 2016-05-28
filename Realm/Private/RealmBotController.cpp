#include "Realm.h"
#include "RealmBotController.h"

ARealmBotController::ARealmBotController(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	bWantsPlayerState = true;
}