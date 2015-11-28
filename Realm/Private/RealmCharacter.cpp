#include "Realm.h"
#include "RealmCharacter.h"
#include "UnrealNetwork.h"

ARealmCharacter::ARealmCharacter(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void ARealmCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmCharacter, playerController);
}