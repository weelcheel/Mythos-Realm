// Fill out your copyright notice in the Description page of Project Settings.

#include "Realm.h"
#include "RealmGameMode.h"
#include "RealmPlayerController.h"
#include "SpectatorCharacter.h"
#include "Mod.h"
#include "RealmEnabler.h"
#include "RealmPlayerState.h"
#include "PlayerCharacter.h"
#include "LaneManager.h"
#include "GameCharacter.h"
#include "RealmPlayerStart.h"
#include "RealmFogofWarManager.h"
#include "RealmGameInstance.h"
#include "RealmGameState.h"
#include "RealmObjective.h"
#include "RealmForestMinionCamp.h"
#include "RealmTurret.h"
#include "RealmMoveController.h"

ARealmGameMode::ARealmGameMode(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	PlayerControllerClass = ARealmPlayerController::StaticClass();
	DefaultPawnClass = ASpectatorCharacter::StaticClass();
	PlayerStateClass = ARealmPlayerState::StaticClass();

	//Blueprint'/Game/Realm/UI/PlayerHUD/PlayerHUD.PlayerHUD'
	/*static ConstructorHelpers::FClassFinder<AHUD> hudClass(TEXT("/Game/Realm/UI/PlayerHUD/SandboxHUD"));
	if (hudClass.Class != NULL)
	{
		HUDClass = hudClass.Class;
	}*/

	teamCount = 2;
	teamSizeMax = 5;

	for (int32 i = 0; i < teamCount; i++)
	{
		FTeam newTeam;

		teams.Add(newTeam);
	}

	bDelayedStart = true;
	bRankedGame = true;

	ambientLevelUpTime = 130.f;
}

void ARealmGameMode::StartMatch()
{
	Super::StartMatch();

	for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
	{
		(*laneitr)->MatchStarted();
	}

	for (TActorIterator<AForestCamp> foritr(GetWorld()); foritr; ++foritr)
	{
		AForestCamp* fc = (*foritr);
		if (IsValid(fc))
		{
			FTimerHandle i;
			GetWorldTimerManager().SetTimer(i, fc, &AForestCamp::SpawnMinions, 15.f, false);
		}
	}

	FTimerHandle h;
	GetWorldTimerManager().SetTimer(h, this, &ARealmGameMode::StartCreditIncome, 5.f, false);

	FTimerHandle j;
	GetWorldTimerManager().SetTimer(j, this, &ARealmGameMode::AmbientGameLevelUp, ambientLevelUpTime, true);

	ARealmGameState* gs = GetGameState<ARealmGameState>();
	if (IsValid(gs))
	{
		for (int32 i = 0; i < teams.Num(); i++)
			gs->teamScores.Add(0);

		gs->matchStartTime = GetWorld()->GetTimeSeconds();
	}
}

void ARealmGameMode::RestartPlayer(class AController* NewPlayer)
{
	if (NewPlayer == NULL || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if (NewPlayer->PlayerState && NewPlayer->PlayerState->bOnlySpectator)
	{
		return;
	}

	AActor* StartSpot = FindPlayerStart(NewPlayer);

	// if a start spot wasn't found,
	if (StartSpot == NULL)
	{
		// check for a previously assigned spot
		if (NewPlayer->StartSpot != NULL)
		{
			StartSpot = NewPlayer->StartSpot.Get();
		}
		else
		{
			// otherwise abort
			return;
		}
	}
	// try to create a pawn to use of the default class for this player
	if (NewPlayer->GetPawn() == NULL && GetDefaultPawnClassForController(NewPlayer) != NULL)
	{
		NewPlayer->SetPawn(SpawnDefaultPawnFor(NewPlayer, StartSpot));
	}

	if (NewPlayer->GetPawn() == NULL)
	{
		NewPlayer->FailedToSpawnPawn();
	}
	else
	{
		// initialize and start it up
		InitStartSpot(StartSpot, NewPlayer);

		// @todo: this was related to speedhack code, which is disabled.
		/*
		if ( NewPlayer->GetAPlayerController() )
		{
		NewPlayer->GetAPlayerController()->TimeMargin = -0.1f;
		}
		*/
		NewPlayer->Possess(NewPlayer->GetPawn());

		// If the Pawn is destroyed as part of possession we have to abort
		if (NewPlayer->GetPawn() == nullptr)
		{
			NewPlayer->FailedToSpawnPawn();
		}
		else
		{
			// set initial control rotation to player start's rotation
			NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);

			FRotator NewControllerRot = StartSpot->GetActorRotation();
			NewControllerRot.Roll = 0.f;
			NewPlayer->SetControlRotation(NewControllerRot);

			SetPlayerDefaults(NewPlayer->GetPawn());

			K2_OnRestartPlayer(NewPlayer);
		}
	}

#if !UE_WITH_PHYSICS
	if (NewPlayer->GetPawn() != NULL)
	{
		UCharacterMovementComponent* CharacterMovement = Cast<UCharacterMovementComponent>(NewPlayer->GetPawn()->GetMovementComponent());
		if (CharacterMovement)
		{
			CharacterMovement->bCheatFlying = true;
			CharacterMovement->SetMovementMode(MOVE_Flying);
		}
	}
#endif	//!UE_WITH_PHYSICS
}

void ARealmGameMode::StartCreditIncome()
{
	for (TActorIterator<APlayerCharacter> plyitr(GetWorld()); plyitr; ++plyitr)
	{
		(*plyitr)->StartAmbientCreditIncome(ambientCreditIncome);
	}
}

void ARealmGameMode::GetStoreMods(TArray<TSubclassOf<AMod> >& modsToSell)
{
	modsToSell = storeMods;
}

void ARealmGameMode::BeginPlay()
{
	Super::BeginPlay();

	gameStatus = EGameStatus::GS_Pregame;

	uint32 epn = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("epn"), epn))
	{
		expectedPlayerCount = epn;
		teamSizeMax = expectedPlayerCount / teamCount;
	}
	else
		expectedPlayerCount = 1;

	if (sightManagers.Num() <= 0)
	{
		for (FTeam inTeam : teams)
		{
			ARealmFogofWarManager* mg = GetWorld()->SpawnActor<ARealmFogofWarManager>(ARealmFogofWarManager::StaticClass());
			if (IsValid(mg))
			{
				mg->teamIndex = sightManagers.AddUnique(mg);
			}
		}
	}
}

void ARealmGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (sightManagers.Num() <= 0)
	{
		for (FTeam inTeam : teams)
		{
			ARealmFogofWarManager* mg = GetWorld()->SpawnActor<ARealmFogofWarManager>(ARealmFogofWarManager::StaticClass());
			if (IsValid(mg))
			{
				mg->teamIndex = sightManagers.AddUnique(mg);
			}
		}
	}

	ARealmPlayerController* pc = Cast<ARealmPlayerController>(NewPlayer);
	if (IsValid(pc))
	{
		pc->ClientInitIngameStore(storeMods);
	}

	//we should only be logging in while the server is in pregame, any other time and they're spectators
	if (gameStatus == EGameStatus::GS_Pregame || gameStatus == EGameStatus::GS_CharacterSelect)
	{
		//@todo: whether or not the player has a party

		int32 least = 0;
		for (int32 i = 0; i < teams.Num(); i++)
		{
			if (teams[i].players.Num() < teams[least].players.Num())
				least = i;
		}

		if (least >= 0 && teams[least].players.Num() + 1 <= teamSizeMax)
		{
			ARealmPlayerState* ps = Cast<ARealmPlayerState>(pc->PlayerState);
			if (ps)
			{
				ps->SetTeamIndex(least);
				teams[least].players.AddUnique(ps);
				ps->SetTeamPlayerIndex(teams[least].players.Num() - 1);
				sightManagers[least]->AddPlayerToManager(pc);
			}
		}

		if (IsValid(pc))
			pc->ClientOpenPregameScreen();

		CheckForCharacterSelect();
	}
}

void ARealmGameMode::EnablerDestroyed(ARealmEnabler* enablerDestroyed, int32 winningTeam)
{
	if (!IsValid(enablerDestroyed))
		return;

	if (!bGameWinner)
	{
		bGameWinner = true;
		
		winningTeamIndex = winningTeam;
		EndRealmMatch();

		UE_LOG(LogTemp, Warning, TEXT("TEAM %d HAS WON THE MATCH!"), winningTeam);
	}
}

bool ARealmGameMode::CanDamageFriendlies() const
{
	return bFriendlyFire;
}

void ARealmGameMode::CheckForCharacterSelect()
{
	if (gameStatus > EGameStatus::GS_Pregame)
		return;

	int32 playerCount = 0; 
	for (int32 i = 0; i < teams.Num(); i++)
		playerCount += teams[i].players.Num();

	if (playerCount >= expectedPlayerCount)
	{
		gameStatus = EGameStatus::GS_CharacterSelect;
		FTimerHandle h;

		GetWorldTimerManager().SetTimer(h, this, &ARealmGameMode::TransitionToCharacterSelect, 3.5f, false);
	}
}

void ARealmGameMode::TransitionToCharacterSelect()
{
	for (FConstPlayerControllerIterator plyrItr = GetWorld()->GetPlayerControllerIterator(); plyrItr; ++plyrItr)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyrItr));
		if (IsValid(pc))
			pc->ClientOpenCharacterSelectScreen(availableCharacters);
	}
}

void ARealmGameMode::PlayerSelectedCharacter(ARealmPlayerController* player, TSubclassOf<APlayerCharacter> characterClass)
{
	if (!availableCharacters.Contains(characterClass))
		return;
	if (!IsValid(player))
		return;

	ARealmPlayerState* ps = Cast<ARealmPlayerState>(player->PlayerState);
	if (IsValid(ps))
		ps->SetChosenCharacterClass(characterClass);

	CheckForAllCharactersSelected();
}

void ARealmGameMode::CheckForAllCharactersSelected()
{
	bool bPlayerNoSelect = false;
	for (FConstPlayerControllerIterator plyrItr = GetWorld()->GetPlayerControllerIterator(); plyrItr; ++plyrItr)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyrItr));
		if (IsValid(pc) && !bPlayerNoSelect)
			bPlayerNoSelect = pc->GetDefaultCharacterClass() == nullptr;
	}

	if (!bPlayerNoSelect)
	{
		StartMatch();

		for (FConstPlayerControllerIterator plyrItr = GetWorld()->GetPlayerControllerIterator(); plyrItr; ++plyrItr)
		{
			ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyrItr));
			if (IsValid(pc))
				pc->ClientOpenPlayerHUD();
		}
	}
}

void ARealmGameMode::PlayerDied(AController* killedPlayer, AController* playerKiller, APawn* killerPawn)
{
	if (!IsValid(killedPlayer))
		return;

	ARealmPlayerController* killedPC = Cast<ARealmPlayerController>(killedPlayer);
	ARealmGameState* gs = GetGameState<ARealmGameState>();
	if (IsValid(killedPC))
	{
		//award the killer
		ARealmPlayerController* killerPC = Cast<ARealmPlayerController>(playerKiller);
		if (IsValid(killerPC))
		{
			//award the killer
			APlayerCharacter* pc = killerPC->GetPlayerCharacter();
			if (IsValid(pc))
				pc->ChangeCredits(CalculatePlayerKillValue(killedPlayer, playerKiller), killedPC->GetPlayerCharacter()->GetActorLocation());
		}

		//announce the kill to the game
		ARealmPlayerState* ps = Cast<ARealmPlayerState>(killedPC->PlayerState);
		ARealmPlayerState* ps2 = nullptr;

		if (IsValid(killerPC))
			ps2 = Cast<ARealmPlayerState>(killerPC->PlayerState);
		if (IsValid(ps))
		{
			ps->playerDeaths++;
			ps->BroadcastDeath(ps2, killerPawn);
		}
		if (IsValid(ps2))
		{
			ps2->playerKills++;
			if (IsValid(gs))
			{
				if (ps2->GetTeamIndex() >= 0 && ps2->GetTeamIndex() < gs->teamScores.Num())
					gs->teamScores[ps2->GetTeamIndex()]++;
			}
		}

		//award the assistors

		//set the respawn timer for the killed
		if (IsValid(gs) && IsValid(killedPC->GetPlayerCharacter()))
		{
			float respawnTime = 7.5f;
			respawnTime += FMath::Min((gs->GetMatchTime() / 300.f), 35.f) + (float)killedPC->GetPlayerCharacter()->level * 1.5f;
			killedPC->GetPlayerCharacter()->StartRespawnTimers(respawnTime);
		}
	}
}

void ARealmGameMode::ObjectiveDestroyed(ARealmObjective* destroyedObjective, APawn* killerPawn)
{
	AGameCharacter* gc = Cast<AGameCharacter>(killerPawn);
	if (!IsValid(destroyedObjective) || !IsValid(gc))
		return;

	//award the players
	for (TActorIterator<APlayerCharacter> plyitr(GetWorld()); plyitr; ++plyitr)
	{
		APlayerCharacter* pc = (*plyitr);
		if (IsValid(pc) && pc->GetTeamIndex() == gc->GetTeamIndex())
			pc->ChangeCredits(destroyedObjective->playerReward, destroyedObjective->GetActorLocation());
	}

	//notify the players
	ARealmGameState* gs = Cast<ARealmGameState>(GameState);
	if (IsValid(gs))
		gs->BroadcastObjectiveDeath(killerPawn, destroyedObjective);
}

AActor* ARealmGameMode::FindPlayerStart(AController* Player, const FString& IncomingName)
{
	ARealmPlayerStart* ps = nullptr;
	for (TActorIterator<ARealmPlayerStart> plyitr(GetWorld()); plyitr; ++plyitr)
	{
		ps = (*plyitr);
		ARealmPlayerState* pls = Cast<ARealmPlayerState>(Player->PlayerState);
		if (IsValid(ps) && IsValid(pls) && ps->teamIndex == pls->GetTeamIndex() && ps->indSpawnIndex == pls->GetTeamPlayerIndex())
			return ps;
	}

	return ps;
}

int32 ARealmGameMode::CalculatePlayerKillValue(AController* killedPlayer, AController* killerPlayer)
{
	return 350;
}

void ARealmGameMode::EndRealmMatch()
{
	/*for (TActorIterator<ARealmFogofWarManager> fogitr(GetWorld()); fogitr; ++fogitr)
		GetWorldTimerManager().ClearAllTimersForObject((*fogitr));
	for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
	{
		GetWorldTimerManager().ClearAllTimersForObject((*laneitr));
	}

	for (TActorIterator<AGameCharacter> gamechr(GetWorld()); gamechr; ++gamechr)
	{
		(*gamechr)->StopAutoAttack();
		if (IsValid((*gamechr)->GetController()))
			(*gamechr)->GetController()->StopMovement();

		GetWorldTimerManager().ClearAllTimersForObject((*gamechr));
	}*/

	CalculateEndgame();
	for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
	{
		GetWorldTimerManager().ClearAllTimersForObject(*laneitr);
	}

	for (FConstPlayerControllerIterator plyr = GetWorld()->GetPlayerControllerIterator(); plyr; ++plyr)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyr));
		if (IsValid(pc))
		{
			//pc->GameEnded();
			pc->ClientOpenEndgameUI(winningTeamIndex);
		}
	}

	for (TActorIterator<ARealmMoveController> mvCtr(GetWorld()); mvCtr; ++mvCtr)
	{
		ARealmMoveController* mc = (*mvCtr);
		if (IsValid(mc))
			mc->GameEnded();
	}
}

void ARealmGameMode::CalculateEndgame()
{
	for (FConstPlayerControllerIterator plyr = GetWorld()->GetPlayerControllerIterator(); plyr; ++plyr)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyr));
		if (IsValid(pc))
			pc->ClientSendEndgameUserID();
	}

	GetWorldTimerManager().SetTimer(useridcheck, this, &ARealmGameMode::CheckForEndgameIDs, 0.05f, true);
}

void ARealmGameMode::CheckForEndgameIDs()
{
	int32 count = 0;
	for (FConstPlayerControllerIterator plyr = GetWorld()->GetPlayerControllerIterator(); plyr; ++plyr)
		count++;

	if (endgameUserids.Num() == count)
	{
		GetWorldTimerManager().ClearTimer(useridcheck);
		ReportEndGame();
	}
}

void ARealmGameMode::ReportEndGame()
{
	URealmGameInstance* instance = Cast<URealmGameInstance>(GetGameInstance());
	if (instance)
		instance->SendMatchComplete(this);
}

void ARealmGameMode::ReceiveEndgameStats(const FString& userid, int32 teamIndex)
{
	endgameTeams.Add(teamIndex);
	endgameUserids.AddUnique(userid);
}

void ARealmGameMode::PlayerLeveledUp()
{
	for (int32 i = 0; i < teams.Num(); i++)
	{
		int32 levels = 0;
		int32 count = 0;
		for (TActorIterator<APlayerCharacter> plyitr(GetWorld()); plyitr; ++plyitr)
		{
			APlayerCharacter* pc = (*plyitr);
			if (IsValid(pc) && pc->GetTeamIndex() == i)
			{
				levels += pc->GetLevel();
				count++;
			}
		}

		if (count <= 0)
			count++;

		int32 avg = levels / count;

		for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
		{
			ALaneManager* lm = (*laneitr);
			if (IsValid(lm) && lm->teamIndex == i)
				lm->SetMinionLevel(avg);
		}
	}
}

void ARealmGameMode::AmbientGameLevelUp()
{
	//lane minions
	for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
	{
		ALaneManager* lm = (*laneitr);
		if (IsValid(lm))
			lm->SetMinionLevel(lm->spawnMinionLevel + 1);
	}

	//turrets
	for (TActorIterator<ATurret> turritr(GetWorld()); turritr; ++turritr)
	{
		ATurret* tr = (*turritr);
		if (IsValid(tr) && tr->IsAlive())
			tr->LevelUp();
	}
}