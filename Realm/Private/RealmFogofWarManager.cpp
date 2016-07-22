#include "Realm.h"
#include "RealmFogofWarManager.h"
#include "GameCharacter.h"
#include "PlayerCharacter.h"
#include "RealmPlayerController.h"
#include "RealmGameMode.h"

URealmFogofWarManager::URealmFogofWarManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	
}

void URealmFogofWarManager::StartCalculatingVisibility()
{
	if (IsValid(playerOwner) && playerOwner->HasAuthority())
		playerOwner->GetWorldTimerManager().SetTimer(visibilityTimer, this, &URealmFogofWarManager::CalculateTeamVisibility, (0.15f), true);
	else if (IsValid(gameOwner))
		gameOwner->GetWorldTimerManager().SetTimer(visibilityTimer, this, &URealmFogofWarManager::CalculateTeamVisibility, (0.15f), true);

	FGameVisibilityWorker::WorkerInit(this);
}

void URealmFogofWarManager::CalculateTeamVisibility()
{
	UWorld* gameWorld = IsValid(playerOwner) ? playerOwner->GetWorld() : gameOwner->GetWorld();

	if (availableUnits.Num() <= 0)
	{
		for (TActorIterator<AGameCharacter> itr(gameWorld); itr; ++itr)
			availableUnits.AddUnique(*itr);
	}

	//update the players with their new sight lists
	for (FConstPlayerControllerIterator Iterator = gameWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>(*Iterator);
		if (IsValid(pc) && IsValid(pc->GetPlayerCharacter()) && enemySightLists[pc->GetPlayerCharacter()->GetTeamIndex()].sightList.Num() > 0)
			pc->sightList = enemySightLists[pc->GetPlayerCharacter()->GetTeamIndex()].sightList;
	}
}

void URealmFogofWarManager::AddCharacterToManager(AGameCharacter* newCharacter)
{
	if (IsValid(newCharacter))
		teamCharacters.AddUnique(newCharacter);
}

void URealmFogofWarManager::RemoveCharacterFromManager(AGameCharacter* oldCharacter)
{
	if (IsValid(oldCharacter))
		teamCharacters.Remove(oldCharacter);
}

void URealmFogofWarManager::AddPlayerToManager(ARealmPlayerController* newPlayer)
{
	//if (IsValid(newPlayer))
		//teamPlayers.AddUnique(newPlayer);
}

bool URealmFogofWarManager::CanUnitSeeOther(AGameCharacter* originUnit, AGameCharacter* testUnit) const
{
	if (!IsValid(originUnit) || !IsValid(testUnit))
		return false;

	return enemySightLists[originUnit->GetTeamIndex()].sightList.Contains(testUnit);
}

void URealmFogofWarManager::BeginDestroy()
{
	Super::BeginDestroy();

	FGameVisibilityWorker::Shutdown();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------
FGameVisibilityWorker* FGameVisibilityWorker::runnable = nullptr;

FGameVisibilityWorker::FGameVisibilityWorker(URealmFogofWarManager* inFoW, TArray<FTeamSightList>& sightLists)
: fogOfWar(inFoW)
{
	enemySightLists = &sightLists;

	Thread = FRunnableThread::Create(this, TEXT("FGameVisibilityWorker"), 0, TPri_BelowNormal);
}

FGameVisibilityWorker::~FGameVisibilityWorker()
{
	delete Thread;
	Thread = nullptr;
}

bool FGameVisibilityWorker::Init()
{
	return true;
}

uint32 FGameVisibilityWorker::Run()
{
	while (stopTaskCounter.GetValue() == 0)
	{
		CalculateVisibilities(); //actually calculate visibilities
		FPlatformProcess::Sleep(0.05f); //sleep for a period of time to let the game world update
	}

	return 0;
}

void FGameVisibilityWorker::Stop()
{
	stopTaskCounter.Increment();
}

FGameVisibilityWorker* FGameVisibilityWorker::WorkerInit(URealmFogofWarManager* inFoW)
{
	if (!runnable && FPlatformProcess::SupportsMultithreading() && inFoW)
		runnable = new FGameVisibilityWorker(inFoW, inFoW->enemySightLists);

	return runnable;
}

void FGameVisibilityWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FGameVisibilityWorker::Shutdown()
{
	if (runnable)
	{
		runnable->EnsureCompletion();
		delete runnable;
		runnable = nullptr;
	}
}

void FGameVisibilityWorker::CalculateVisibilities()
{
	if (!IsValid(fogOfWar) || !fogOfWar->IsValidLowLevelFast() || (!IsValid(fogOfWar->playerOwner) && !IsValid(fogOfWar->gameOwner)))
		return;

	//don't calculate if we weren't given available units and just wait until we do
	if (fogOfWar->availableUnits.Num() <= 0)
		return;

	UWorld* gameWorld = IsValid(fogOfWar->playerOwner) ? fogOfWar->playerOwner->GetWorld() : fogOfWar->gameOwner->GetWorld();

	if (!gameWorld)
		return;

	//clear enemy sight lists
	for (int32 list = 0; list < enemySightLists->Num(); list++)
		(*enemySightLists)[list].sightList.Empty();

	for (AGameCharacter* gc : fogOfWar->availableUnits)
	{
		//get their sight data if they're alive
		if (gc->IsAlive())
			gc->CalculateVisibility((*enemySightLists)[gc->GetTeamIndex()].sightList, fogOfWar->availableUnits);
	}

	fogOfWar->availableUnits.Empty();
}