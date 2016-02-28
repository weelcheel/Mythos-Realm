#include "Realm.h"
#include "RealmMainMenu.h"
#include "RealmGameInstance.h"

ARealmMainMenu::ARealmMainMenu(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	//pause spawning any pawns
	bDelayedStart = true;
}

void ARealmMainMenu::BeginPlay()
{
	Super::BeginPlay();
}

void ARealmMainMenu::AttemptLogin(FString username, FString password)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		gi->AttemptLogin(username, password);
}

void ARealmMainMenu::AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		gi->AttemptCreateLogin(username, password, email, ingameAlias);
}

void ARealmMainMenu::AttemptJoinMatchmakingSolo(const FString& queue)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		gi->AttemptJoinSoloMMQueue(queue);
}

void ARealmMainMenu::PlayerConfirmMatch(const FString& matchID)
{
	URealmGameInstance* gi = Cast<URealmGameInstance>(GetGameInstance());
	if (gi)
		gi->SendConfirmMatch(matchID);
}