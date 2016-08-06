#include "Realm.h"
#include "RealmObjective.h"

ARealmObjective::ARealmObjective(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void ARealmObjective::CheckDamage(FTakeHitInfo& damage)
{

}

void ARealmObjective::GiveCharacterExperience(int32 amount)
{
	//don't do anything; objectives don't receive experience
}