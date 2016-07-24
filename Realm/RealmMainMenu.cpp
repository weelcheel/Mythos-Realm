#include "Realm.h"
#include "RealmMainMenu.h"
#include "RealmGameInstance.h"
#include "RealmPlayerState.h"

ARealmMainMenu::ARealmMainMenu(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	//pause spawning any pawns
	PlayerStateClass = ARealmPlayerState::StaticClass();

	bDelayedStart = true;
}

void ARealmMainMenu::BeginPlay()
{
	Super::BeginPlay();
}

bool ARealmMainMenu::AttemptLogin(FString username, FString password)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		return gi->AttemptLogin(username, password);

	return false;
}

bool ARealmMainMenu::AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias, FString alphaCode)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		return gi->AttemptCreateLogin(username, password, email, ingameAlias, alphaCode);
	
	return false;
}

bool ARealmMainMenu::AttemptJoinMatchmakingSolo(const FString& queue)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		return gi->AttemptJoinSoloMMQueue(queue);

	return false;
}

bool ARealmMainMenu::PlayerConfirmMatch(const FString& matchID)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		return gi->SendConfirmMatch(matchID);

	return false;
}