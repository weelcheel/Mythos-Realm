#include "Realm.h"
#include "RealmSocketListener.h"
#include "RealmGameInstance.h"

FRealmSocketListener::FRealmSocketListener(FSocket* socketToListenTo, bool LoginSocket)
{
	listenSocket = socketToListenTo;
	bLoginSocket = LoginSocket;
	listenerThread = FRunnableThread::Create(this, TEXT("FRealmSocketListener"));
}

FRealmSocketListener::~FRealmSocketListener()
{
	delete listenerThread;
	listenerThread = nullptr;
}

void FRealmSocketListener::ListenForData()
{

}

bool FRealmSocketListener::Init()
{
	return true;
}

uint32 FRealmSocketListener::Run()
{
	while (stopListenerThread.GetValue() == 0)
	{
		if (!listenSocket)
			continue;

		TArray<uint8> ReceivedData;

		uint32 Size;
		while (listenSocket->HasPendingData(Size))
		{
			ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));

			int32 Read = 0;
			listenSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (ReceivedData.Num() <= 0)
		{
			//No Data Received
			continue;
		}

		dataQueue.Enqueue(ReceivedData);

		//Initial wait before starting
		FPlatformProcess::Sleep(0.03);
	}

	return 0;
}

void FRealmSocketListener::Stop()
{
	stopListenerThread.Increment();
}

void FRealmSocketListener::EnsureCompletion()
{
	Stop();
	listenerThread->WaitForCompletion();
}

FRealmSocketListener* FRealmSocketListener::CreateListener(FSocket* socketToListenTo, bool LoginSocket)
{
	if (FPlatformProcess::SupportsMultithreading())
		return new FRealmSocketListener(socketToListenTo, LoginSocket);
	else
		return nullptr;
}

void FRealmSocketListener::GetNextDataArray(TArray<uint8>& outArray)
{
	TArray<uint8> data;

	if (!dataQueue.IsEmpty())
		dataQueue.Dequeue(data);

	outArray = data;
}