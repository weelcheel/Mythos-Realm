#pragma once

#include "Navigation/CrowdFollowingComponent.h"
#include "RealmCrowdComponent.generated.h"

UCLASS()
class URealmCrowdComponent : public UCrowdFollowingComponent
{
	friend class ARealmMoveController;

	GENERATED_UCLASS_BODY()

};