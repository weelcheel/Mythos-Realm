#include "Realm.h"
#include "RealmGameInstance.h"
#include <string>
#include "RealmMainMenu.h"

URealmGameInstance::URealmGameInstance(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

bool URealmGameInstance::StartTCPReceiver(
	const FString& YourChosenSocketName,
	const FString& TheIP,
	const int32 ThePort
	)
{
	if (!IsValid(GetWorld()))
		return false;

	listenerSocket = CreateTCPConnectionListener(YourChosenSocketName, TheIP, ThePort);

	//Not created?
	if (!listenerSocket)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StartTCPReceiver>> Listen socket could not be created! ~> %s %d"), *TheIP, ThePort));
		return false;
	}

	//Start the Listener! //thread this eventually
	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, this,
		&URealmGameInstance::TCPConnectionListener, 0.01, true);

	return true;
}

bool URealmGameInstance::StartTCPReceiver(FSocket* loginSocket)
{
	if (!IsValid(GetWorld()))
		return false;

	connectionSocket = loginSocket;

	if (!connectionSocket)
		return false;

	FTimerHandle handle;

	GetWorld()->GetTimerManager().SetTimer(handle, this,
		&URealmGameInstance::TCPSocketListener, 0.01, true);

	return true;
}

FSocket* URealmGameInstance::CreateTCPConnectionListener(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, const int32 ReceiveBufferSize)
{
	int32 port = 3308;
	FIPv4Address ip;
	FIPv4Address::Parse(TheIP, ip);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.GetValue());
	addr->SetPort(port);
	FSocket* ListenSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("loginSocket"), false);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(ReceiveBufferSize, NewSize);

	//Done!
	return ListenSocket;
}

void URealmGameInstance::TCPConnectionListener()
{
	//~~~~~~~~~~~~~
	if (!listenerSocket) return;

	FTimerHandle handle;
	//~~~~~~~~~~~~~

	//Remote address
	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;

	// handle incoming connections
	if (listenerSocket->HasPendingConnection(Pending) && Pending)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//Already have a Connection? destroy previous
		if (connectionSocket)
		{
			connectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(connectionSocket);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//New Connection receive!
		connectionSocket = listenerSocket->Accept(*RemoteAddress, TEXT("Received Socket Connection"));

		if (connectionSocket != NULL)
		{
			//Global cache of current Remote Address
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);

			//UE_LOG "Accepted Connection! WOOOHOOOO!!!";

			//can thread this too
			GetWorld()->GetTimerManager().SetTimer(handle, this,
				&URealmGameInstance::TCPSocketListener, 0.01, true);
		}
	}
}

FString URealmGameInstance::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

void URealmGameInstance::TCPSocketListener()
{
	//~~~~~~~~~~~~~
	if (!connectionSocket) return;
	//~~~~~~~~~~~~~


	//Binary Array!
	TArray<uint8> ReceivedData;

	uint32 Size;
	while (connectionSocket->HasPendingData(Size))
	{
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));

		int32 Read = 0;
		connectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Read! %d"), ReceivedData.Num()));
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

		//multiplayer lobby parsing

		connectionSocket->Close();
	}

	//VShow("As String!!!!! ~>", ReceivedUE4String);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
}

void URealmGameInstance::AttemptLogin(FString username, FString password)
{
	//try to establish a connection to the login server
	loginSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("login"), false);

	FString address = TEXT("192.168.1.4");
	int32 port = 3308;
	FIPv4Address ip;
	FIPv4Address::Parse(address, ip);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.GetValue());
	addr->SetPort(port);

	bool connected = loginSocket->Connect(*addr);
	UE_LOG(LogTemp, Warning, TEXT("trying to connect"));

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
	bool bSuccesfullySentLogin = loginSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	UE_LOG(LogTemp, Warning, TEXT("tried to login and sent %d bytes to the server"), sent);

	if (!StartTCPReceiver(loginSocket))
		return;
}

void URealmGameInstance::AttemptCreateLogin(FString username, FString password, FString email, FString ingameAlias)
{
	//try to establish a connection to the login server
	loginSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("login"), false);

	FString address = TEXT("192.168.1.4");
	int32 port = 3308;
	FIPv4Address ip;
	FIPv4Address::Parse(address, ip);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.GetValue());
	addr->SetPort(port);

	bool connected = loginSocket->Connect(*addr);
	UE_LOG(LogTemp, Warning, TEXT("trying to connect"));

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
	bool bSuccesfullySentLogin = loginSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), encSize, sent);
	UE_LOG(LogTemp, Warning, TEXT("tried to create a login and sent %d bytes to the server"), sent);

	if (!StartTCPReceiver(loginSocket))
		return;
}