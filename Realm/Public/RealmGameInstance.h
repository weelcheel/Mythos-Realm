#pragma once

#include "Realm.h"
//#include "OnlineIdentityInterface.h"
//#include "OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "RankDivisions.h"
#include "Networking.h"
#include "RealmGameInstance.generated.h"

UCLASS()
class URealmGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()

protected:

	/* userid that is currently signed in */
	UPROPERTY(BlueprintReadOnly, Category = RealmInstance)
	FString currentUserid;

	/* alias name of the player to be shown in the menus */
	UPROPERTY(BlueprintReadOnly, Category = RealmInstance)
	FString currentAlias;

	/* current amount of mythos points for the user */
	UPROPERTY(BlueprintReadOnly, Category = RealmInstance)
	int32 currentMythosPoints;

	/* current league this player is in (calculated from mythos points) */
	UPROPERTY(BlueprintReadOnly, Category = RealmInstance)
	EPlayerLeague currentPlayerLeague;

	/* what division (or rank if legend) of the league the player is in (calculated from mythos points) */
	UPROPERTY(BlueprintReadOnly, Category = RealmInstance)
	int32 currentPlayerDivision;

	/* socket connection to the login server */
	FSocket* loginSocket;
	FSocket* listenerSocket;
	FSocket* connectionSocket;
	FIPv4Endpoint RemoteAddressForConnection;

	bool StartTCPReceiver(
		const FString& YourChosenSocketName,
		const FString& TheIP,
		const int32 ThePort
		);

	bool StartTCPReceiver(FSocket* loginSocket);

	FSocket* CreateTCPConnectionListener(
		const FString& YourChosenSocketName,
		const FString& TheIP,
		const int32 ThePort,
		const int32 ReceiveBufferSize = 2 * 1024 * 1024
		);

	void TCPConnectionListener();
	void TCPSocketListener();

	FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);

public:

	/* attempts to contact the login server and perform a login */
	void AttemptLogin(FString username, FString password);

	/* attempts to contact the login server to create a new account with the provided credentials */
	void AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias);
};