#include "Realm.h"
#include "RealmGameInstance.h"
#include <string>
#include "RealmMainMenu.h"

URealmGameInstance::URealmGameInstance(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

URealmGameInstance::~URealmGameInstance()
{
	if (loginSocketThread)
		loginSocketThread->EnsureCompletion();

	if (multiplayerSocketThread)
	multiplayerSocketThread->EnsureCompletion();
}

FString URealmGameInstance::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

bool URealmGameInstance::ConnectLoginSocket()
{
	//try to establish a connection to the login server
	if (!loginSocketThread || !loginSocket || loginSocket->GetConnectionState() != SCS_Connected)
	{
		FSocket* ls = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("login"), false);
		auto resolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName("mythosrealm.ddns.net");

		while (!resolveInfo->IsComplete());

		uint32 outip = 0;
		if (resolveInfo->GetErrorCode() == 0)
		{
			const FInternetAddr* addr = &resolveInfo->GetResolvedAddress();
			addr->GetIp(outip);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("failed to resolve the hostname"));
			return false;
		}

		int32 port = 3308;
		TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		addr->SetIp(outip);
		addr->SetPort(port);

		int32 ReceiveBufferSize = 2 * 1024 * 1024;
		int32 newSize = 0;
		ls->SetReceiveBufferSize(ReceiveBufferSize, newSize);
		ls->SetSendBufferSize(ReceiveBufferSize, newSize);

		bool connected = ls->Connect(*addr);
		if (!connected)
		{
			UE_LOG(LogTemp, Warning, TEXT("failed to connect to the login server"));
			return false;
		}

		loginSocketThread = FRealmSocketListener::CreateListener(ls, true);
		loginSocketThread->gameInstance = this;
		loginSocket = ls;

		if (GetWorld())
			GetWorld()->GetTimerManager().SetTimer(loginSocketListenTimer, this, &URealmGameInstance::ListenLoginSocket, 0.03f, true);

		UE_LOG(LogTemp, Warning, TEXT("connected to the login server"));

		return true;
	}

	return true;
}

bool URealmGameInstance::ConnectMultiplayerSocket()
{
	//try to establish a connection to the multiplayer master server
	if (!multiplayerSocketThread || !multiplayerSocket || multiplayerSocket->GetConnectionState() != SCS_Connected)
	{
		FSocket* ms = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("multiplayer"), false);
		auto resolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName("mythosrealm.ddns.net");

		while (!resolveInfo->IsComplete());

		uint32 outip = 0;
		if (resolveInfo->GetErrorCode() == 0)
		{
			const FInternetAddr* addr = &resolveInfo->GetResolvedAddress();
			addr->GetIp(outip);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("failed to resolve the hostname"));
			return false;
		}

		int32 port = 3310;

		TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		addr->SetIp(outip);
		addr->SetPort(port);

		int32 ReceiveBufferSize = 2 * 1024 * 1024;
		int32 newSize = 0;
		ms->SetReceiveBufferSize(ReceiveBufferSize, newSize);
		ms->SetSendBufferSize(ReceiveBufferSize, newSize);

		bool connected = ms->Connect(*addr);
		if (!connected)
		{
			UE_LOG(LogTemp, Warning, TEXT("failed to connect to the multiplayer server"));
			return false;
		}

		multiplayerSocketThread = FRealmSocketListener::CreateListener(ms, false);
		multiplayerSocketThread->gameInstance = this;
		multiplayerSocket = ms;

		if (GetWorld())
			GetWorld()->GetTimerManager().SetTimer(multiplayerSocketListenTimer, this, &URealmGameInstance::ListenMultiplayerSocket, 0.03f, true);

		UE_LOG(LogTemp, Warning, TEXT("connected to the multiplayer server"));
		return true;
	}

	return true;
}

void URealmGameInstance::ListenLoginSocket()
{
	TArray<uint8> data;

	if (loginSocketThread)
	{
		loginSocketThread->GetNextDataArray(data);
		if (data.Num() > 0)
			ParseLoginSocketData(data);
	}
}

void URealmGameInstance::ListenMultiplayerSocket()
{
	TArray<uint8> data;

	if (multiplayerSocketThread)
	{
		multiplayerSocketThread->GetNextDataArray(data);
		if (data.Num() > 0)
			ParseMultiplayerSocketData(data);
	}
}

void URealmGameInstance::ParseLoginSocketData(const TArray<uint8>& ReceivedData)
{
	if (ReceivedData.Num() <= 0)
	{
		//No Data Received
		return;
	}

	//VShow("Total Data read!", ReceivedData.Num());
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Bytes Read ~> %d"), ReceivedData.Num()));


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ARealmMainMenu* mm = Cast<ARealmMainMenu>(GetWorld()->GetAuthGameMode());
	if (!IsValid(mm))
		return;

	TArray<FString> data;
	ReceivedUE4String.ParseIntoArray(data, TEXT("|"), false);

	if (data.Num() > 0)
	{
		//login parsing
		if (data[0].Equals("loginSuccess"))
		{
			currentUserid = data[2];
			currentAlias = data[8];
			currentMythosPoints = FCString::Atoi(*data[6]);

			mm->PlayerLoginSuccessful(currentUserid, FCString::Atoi(*data[4]), currentMythosPoints, currentAlias);
			UE_LOG(LogTemp, Warning, TEXT("Logged in successfully as %s!"), *data[8]);
		}
		if (data[0].Equals("loginFailure"))
		{
			mm->PlayerLoginNotSuccessful();
			UE_LOG(LogTemp, Warning, TEXT("Failed to login!"));
		}
		if (data[0].Equals("createLoginSuccess"))
		{
			mm->CreatePlayerLoginSuccessful();
			UE_LOG(LogTemp, Warning, TEXT("Created new account successfully!"));
		}
		if (data[0].Equals("createLoginFailure"))
		{
			mm->CreatePlayerLoginUnsuccessful(data[1]);
			UE_LOG(LogTemp, Warning, TEXT("Couldn't create new account. Reason: %s"), *data[1]);
		}
		if (data[0].Equals("updateInfo"))
		{
			ReceiveInfoUpdate(data[1], FCString::Atoi(*data[2]));
			UE_LOG(LogTemp, Warning, TEXT("Received info update"));
		}

		//multiplayer lobby parsing

		//connectionSocket->Close();
	}

	//VShow("As String!!!!! ~>", ReceivedUE4String);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
}

void URealmGameInstance::ParseMultiplayerSocketData(const TArray<uint8>& ReceivedData)
{
	if (ReceivedData.Num() <= 0)
	{
		//No Data Received
		return;
	}

	ARealmMainMenu* mm = Cast<ARealmMainMenu>(GetWorld()->GetAuthGameMode());
	if (!IsValid(mm))
		return;

	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	TArray<FString> data;
	ReceivedUE4String.ParseIntoArray(data, TEXT("|"), false);

	if (data.Num() > 0)
	{
		if (data[0].Equals("joinedQueueSuccessfully"))
		{
			mm->JoinMMQueueSuccessful();
			UE_LOG(LogTemp, Warning, TEXT("Joined the MM queue successfully "));
		}
		if (data[0].Equals("joinedQueueFailed"))
		{
			mm->JoinMMQueueFailed();
			UE_LOG(LogTemp, Warning, TEXT("Joined the MM queue failed "));
		}
		if (data[0].Equals("foundMatch"))
		{
			mm->FoundMatch(data[1]);
			UE_LOG(LogTemp, Warning, TEXT("MM found a match "));
		}
		if (data[0].Equals("foundMatchConfirmed"))
		{
			mm->FoundConfirmedMatch(data[1]);
			UE_LOG(LogTemp, Warning, TEXT("MM found a confirmed match"));
		}
		if (data[0].Equals("matchConfirmFailed"))
		{
			mm->FailedToConfirmMatch();
			UE_LOG(LogTemp, Warning, TEXT("MM failed to confirm a match "));
		}
	}
}

bool URealmGameInstance::AttemptLogin(FString username, FString password)
{
	if (!ConnectLoginSocket())
		return false;

	//encode username
	FString serialized = "login|username|";
	serialized = serialized + username;

	//encode encrypted password
	FString serialized1 = "|password|";

	std::string stdstring(TCHAR_TO_UTF8(*password));
	FSHA1 hashState;
	hashState.Update((uint8*)stdstring.c_str(), stdstring.size());
	hashState.Final();

	uint8 passwordHash[FSHA1::DigestSize];
	hashState.GetHash(passwordHash);
	FString pwdStr = BytesToHex(passwordHash, FSHA1::DigestSize);
	serialized1 = serialized1 + pwdStr;

	//get the full login string
	FString prStr = serialized + serialized1;

	//send the encrypted data to the login server
	TCHAR *serializedChar = prStr.GetCharArray().GetData();
	int32 encSize = FCString::Strlen(serializedChar);
	int32 sent = 0;

	//send and store whether or not it was successful
	bool bSuccesfullySent = loginSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	if (bSuccesfullySent)
		UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
	if (!bSuccesfullySent)
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to send"));
		return false;
	}

	return true;
}

bool URealmGameInstance::AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias)
{
	if (!ConnectLoginSocket())
		return false;

	FString sendStr = "loginCreate|username|";
	sendStr += username;
	sendStr += "|password|";

	std::string stdstring(TCHAR_TO_UTF8(*password));
	FSHA1 hashState;
	hashState.Update((uint8*)stdstring.c_str(), stdstring.size());
	hashState.Final();

	uint8 passwordHash[FSHA1::DigestSize];
	hashState.GetHash(passwordHash);
	FString pwdStr = BytesToHex(passwordHash, FSHA1::DigestSize);

	sendStr += pwdStr;
	sendStr += "|email|" + email;
	sendStr += "|ingame|" + ingameAlias;

	//send the encrypted data to the login server
	TCHAR *serializedChar = sendStr.GetCharArray().GetData();
	int32 encSize = FCString::Strlen(serializedChar);
	int32 sent = 0;

	//send and store whether or not it was successful
	bool bSuccesfullySent = loginSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	if (bSuccesfullySent)
		UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
	if (!bSuccesfullySent)
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to send"));
		return false;
	}

	return true;
}

void URealmGameInstance::SendMatchComplete(ARealmGameMode* gameMode)
{
	if (!IsValid(gameMode))
		return;

	if (gameMode->bRankedGame)
	{
		if (!ConnectMultiplayerSocket())
			return;

		FString sendStr = "rankedGameFinished|";
		for (int32 i = 0; i < gameMode->endgameUserids.Num(); i++)
			sendStr += gameMode->endgameUserids[i] + ",";
		sendStr += "|";
		for (int32 j = 0; j < gameMode->endgameTeams.Num(); j++)
			sendStr += FString::FromInt(gameMode->endgameTeams[j]) + ",";
		sendStr += "|" + FString::FromInt(gameMode->winningTeamIndex);

		//send the data to the server
		TCHAR *serializedChar = sendStr.GetCharArray().GetData();
		int32 encSize = FCString::Strlen(serializedChar);
		int32 sent = 0;

		//send and store whether or not it was successful
		bool bSuccesfullySent = multiplayerSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
		if (bSuccesfullySent)
			UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
		if (!bSuccesfullySent)
			UE_LOG(LogTemp, Warning, TEXT("failed to send"));

		FTimerHandle exitTimer;
		gameMode->GetWorldTimerManager().SetTimer(exitTimer, this, &URealmGameInstance::CloseGameInstance, 35.f, false);
	}
}

void URealmGameInstance::CloseGameInstance()
{
	FGenericPlatformMisc::RequestExit(false);
}

FString URealmGameInstance::GetRealmServerIP(int32 port)
{
	auto resolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName("mythosrealm.ddns.net");

	while (!resolveInfo->IsComplete());

	uint32 outip = 0;
	if (resolveInfo->GetErrorCode() == 0)
	{
		const FInternetAddr* addr = &resolveInfo->GetResolvedAddress();
		addr->GetIp(outip);
	}

	FIPv4Address ip = FIPv4Address(outip);
	FString ipstr = ip.ToText().ToString();
	ipstr += ":" + port;

	return ipstr;
}

bool URealmGameInstance::AttemptJoinSoloMMQueue(const FString& queue)
{
	if (!ConnectMultiplayerSocket())
		return false;

	FString sendStr = "playerWantsMMQueue|" + GetUserID() + "|" + queue;

	//send the data to the server
	TCHAR *serializedChar = sendStr.GetCharArray().GetData();
	int32 encSize = FCString::Strlen(serializedChar);
	int32 sent = 0;

	//send and store whether or not it was successful
	bool bSuccesfullySent = multiplayerSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	if (bSuccesfullySent)
		UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
	if (!bSuccesfullySent)
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to send"));
		return false;
	}

	return true;
}

bool URealmGameInstance::SendConfirmMatch(const FString& matchID)
{
	if (!ConnectMultiplayerSocket())
		return false;

	FString sendStr = "playerConfirmMatch|" + GetUserID() + "|" + matchID;

	//send the data to the server
	TCHAR *serializedChar = sendStr.GetCharArray().GetData();
	int32 encSize = FCString::Strlen(serializedChar);
	int32 sent = 0;

	//send and store whether or not it was successful
	bool bSuccesfullySent = multiplayerSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	if (bSuccesfullySent)
		UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
	if (!bSuccesfullySent)
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to send"));
		return false;
	}

	return true;
}

void URealmGameInstance::QueryLoginServerForUpdate()
{
	if (!ConnectLoginSocket())
		return;

	FString sendStr = "getInfoUpdate|" + GetUserID();

	//send the data to the server
	TCHAR *serializedChar = sendStr.GetCharArray().GetData();
	int32 encSize = FCString::Strlen(serializedChar);
	int32 sent = 0;

	//send and store whether or not it was successful
	bool bSuccesfullySent = loginSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	if (bSuccesfullySent)
		UE_LOG(LogTemp, Warning, TEXT("sent %d bytes to the server"), sent);
	if (!bSuccesfullySent)
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to send"));
		return;
	}
}

void URealmGameInstance::ReceiveInfoUpdate(const FString& alias, int32 mp)
{
	currentAlias = alias;
	currentMythosPoints = mp;

	UE_LOG(LogTemp, Warning, TEXT("received info update"));
}