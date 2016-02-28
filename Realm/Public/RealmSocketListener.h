#pragma once

class URealmGameInstance;

class FRealmSocketListener : public FRunnable
{
	FRunnableThread* listenerThread;
	FThreadSafeCounter stopListenerThread;

	FSocket* listenSocket;

	bool bLoginSocket = false;

	void ListenForData();

	/* queue for uobjects to get data from this listener */
	TQueue<TArray<uint8> > dataQueue;

public:

	URealmGameInstance* gameInstance;

	FRealmSocketListener(FSocket* socketToListenTo, bool bLoginSocket);
	virtual ~FRealmSocketListener();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

	void Shutdown();

	void GetNextDataArray(TArray<uint8>& outArray);

	static FRealmSocketListener* CreateListener(FSocket* socketToListenTo, bool bLoginSocket);
};