#include "Realm.h"
#include "Effect.h"
#include "UnrealNetwork.h"

AEffect::AEffect(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AEffect::OnRepDuration()
{
	if (duration > 0.f)
		GetWorldTimerManager().SetTimer(effectTimer, duration, false);
}

void AEffect::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEffect, uiName, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AEffect, description, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AEffect, duration, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AEffect, bStacking, COND_InitialOnly);
	DOREPLIFETIME(AEffect, stackAmount);
}