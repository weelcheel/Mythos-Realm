#pragma once

#include "StatsManager.h"
#include "RealmPlayerController.h"
#include "RealmCharacter.generated.h"

UCLASS(ABSTRACT)
class ARealmCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/* reference to the player controller */
	UPROPERTY(replicated, BlueprintReadOnly, Category=Realm)
	ARealmPlayerController* playerController;

public:

	void SetPlayerController(ARealmPlayerController* newPC)
	{
		playerController = newPC;
	}

	ARealmPlayerController* GetPlayerController() const
	{
		return playerController;
	}
};