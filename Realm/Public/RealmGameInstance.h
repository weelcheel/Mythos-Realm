#pragma once

#include "Realm.h"
//#include "OnlineIdentityInterface.h"
//#include "OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "RankDivisions.h"
#include "Networking.h"
#include "RealmSocketListener.h"
#include "RealmGameInstance.generated.h"

class ARealmGameMode;

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
	FRealmSocketListener* loginSocketThread;
	FRealmSocketListener* multiplayerSocketThread;

	FSocket* loginSocket;
	FSocket* multiplayerSocket;

	FIPv4Endpoint RemoteAddressForConnection;

	FTimerHandle loginSocketListenTimer, multiplayerSocketListenTimer;

	FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);

	void SetupInternetAddresses();

	bool ConnectLoginSocket();
	bool ConnectMultiplayerSocket();
	void ListenLoginSocket();
	void ListenMultiplayerSocket();

	void ReceiveInfoUpdate(const FString& alias, int32 mp);

public:
	~URealmGameInstance();

	/* attempts to contact the login server and perform a login */
	bool AttemptLogin(FString username, FString password);

	/* attempts to contact the login server to create a new account with the provided credentials */
	bool AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias);

	/* attempts to join solo queue */
	bool AttemptJoinSoloMMQueue(const FString& queue);

	/* send match complete info to the database server from a game server */
	void SendMatchComplete(ARealmGameMode* gameMode);

	/* query login server for info update */
	UFUNCTION(BlueprintCallable, Category=Info)
	void QueryLoginServerForUpdate();

	/* send confirm match to the multiplayer server */
	UFUNCTION(BlueprintCallable, Category=MatchConfirm)
	bool SendConfirmMatch(const FString& matchID);

	/* get the current userid */
	const FString& GetUserID() const
	{
		return currentUserid;
	}

	UFUNCTION(BlueprintCallable, Category=Game)
	static FString GetRealmServerIP(int32 port);

	void ParseLoginSocketData(const TArray<uint8>& data);
	void ParseMultiplayerSocketData(const TArray<uint8>& data);

	void CloseGameInstance();
};