#include "Realm.h"
#include "RealmCrowdComponent.h"

URealmCrowdComponent::URealmCrowdComponent(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	bEnableSeparation = true;
	bEnableSlowdownAtGoal = false;

	AvoidanceQuality = ECrowdAvoidanceQuality::Medium;
}