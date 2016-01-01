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
}

void ARealmGameMode::StartMatch()
{
	Super::StartMatch();

	for (TActorIterator<ALaneManager> laneitr(GetWorld()); laneitr; ++laneitr)
	{
		(*laneitr)->MatchStarted();
	}

	FTimerHandle h;

	GetWorldTimerManager().SetTimer(h, this, &ARealmGameMode::StartCreditIncome, 5.f, false);
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

void ARealmGameMode::EnablerDestroyed(ARealmEnabler* enablerDestroyed)
{
	if (!bGameWinner)
	{
		bGameWinner = true;
		if (IsValid(enablerDestroyed))
			UE_LOG(LogTemp, Warning, TEXT("TEAM %d HAS WON THE MATCH!"), enablerDestroyed->GetTeamIndex());
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

	bool bShouldGoToCharacterSelect = false;
	for (int32 i = 0; i < teams.Num(); i++)
		bShouldGoToCharacterSelect = teams[i].players.Num() >= teamSizeMin;

	if (bShouldGoToCharacterSelect)
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

void ARealmGameMode::PlayerDied(ARealmPlayerController* killedPlayer, ARealmPlayerController* playerKiller)
{
	if (!IsValid(killedPlayer) || !IsValid(playerKiller))
		return;

	APlayerCharacter* pc = playerKiller->GetPlayerCharacter();
	if (IsValid(pc))
		pc->ChangeCredits(playerKillWorth);

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