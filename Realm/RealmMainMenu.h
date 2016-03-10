#pragma once

#include "GameFramework/GameMode.h"
#include "RealmMainMenu.generated.h"

UCLASS()
class REALM_API ARealmMainMenu : public AGameMode
{
	GENERATED_UCLASS_BODY()

protected:

	virtual void BeginPlay() override;

public:

	/* attempts to contact the login server and perform a login */
	UFUNCTION(BlueprintCallable, Category=PlayerLogin)
	bool AttemptLogin(FString username, FString password);

	/* attempts to contact the login server to create a new account with the provided credentials */
	UFUNCTION(BlueprintCallable, Category = PlayerLogin)
	bool AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias);

	/* attempts to join the specified matchmaking queue */
	UFUNCTION(BlueprintCallable, Category = PlayerLogin)
	bool AttemptJoinMatchmakingSolo(const FString& queue);

	/* attempts to join the specified matchmaking queue */
	UFUNCTION(BlueprintCallable, Category = PlayerLogin)
	bool PlayerConfirmMatch(const FString& matchID);

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

	/* the player joined matchmaking queue */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void JoinMMQueueSuccessful();

	/* the player failed to join matchmaking */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void JoinMMQueueFailed();

	/* the player found a match and needs to confirm */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void FoundMatch(const FString& matchID);

	/* the player found a confirmed match */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void FoundConfirmedMatch(const FString& matchAddress);

	/* the player failed to accept/confirm the match */
	UFUNCTION(BlueprintImplementableEvent, Category = PlayerLogin)
	void FailedToConfirmMatch();
};