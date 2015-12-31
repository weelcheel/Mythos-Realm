#pragma once

#include "GameFramework/GameMode.h"
#include "Networking.h"
#include "RealmMainMenu.generated.h"

UCLASS()
class REALM_API ARealmMainMenu : public AGameMode
{
	GENERATED_UCLASS_BODY()

protected:

	/* socket connection to the login server */
	FSocket* loginSocket;

	virtual void BeginPlay() override;

	/* server sent back a successful login */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void PlayerLoginSuccessful(const FString& userid, int32 experience, int32 mythosPoints, const FString& alias);

	/* server sent back a failed login */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void PlayerLoginNotSuccessful();

	/* the server successfully created a new account */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void CreatePlayerLoginSuccessful();

	/* the server unsuccessfully created a new account */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void CreatePlayerLoginUnsuccessful(const FString& reason);

public:

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

	/* attempts to contact the login server and perform a login */
	UFUNCTION(BlueprintCallable, Category=PlayerLogin)
	void AttemptLogin(FString username, FString password);

	/* attempts to contact the login server to create a new account with the provided credentials */
	UFUNCTION(BlueprintCallable, Category = PlayerLogin)
	void AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias);
};