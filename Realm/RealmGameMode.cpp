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
}

void ARealmGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ARealmPlayerController* pc = Cast<ARealmPlayerController>(NewPlayer);
	if (IsValid(pc))
	{
		pc->ClientInitIngameStore(storeMods);
	}

	//we should only be logging in while the server is in pregame, any other time and they're spectators
	if (gameStatus == EGameStatus::GS_Pregame)
	{
		//@todo: whether or not the player has a party

		int32 least = -1;
		for (int32 i = 0; i < teams.Num(); i++)
		{
			if (least == -1)
				least = i;
			else
			{
				if (teams[i].players.Num() < teams[least].players.Num())
					least = i;
			}
		}

		if (least >= 0 && teams[least].players.Num() + 1 <= teamSizeMax)
		{
			ARealmPlayerState* ps = Cast<ARealmPlayerState>(pc->PlayerState);
			if (ps)
			{
				ps->SetTeamIndex(least);
				teams[least].players.AddUnique(ps);
				ps->SetTeamPlayerIndex(teams[least].players.Num() - 1);
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
	bool bAllPlayersHaveCharacter = false;
	for (FConstPlayerControllerIterator plyrItr = GetWorld()->GetPlayerControllerIterator(); plyrItr; ++plyrItr)
	{
		ARealmPlayerController* pc = Cast<ARealmPlayerController>((*plyrItr));
		if (IsValid(pc))
		{
			bAllPlayersHaveCharacter = pc->GetDefaultCharacterClass() != nullptr;
			pc->ClientOpenPlayerHUD();
		}
	}

	if (bAllPlayersHaveCharacter)
		StartMatch();
}

void ARealmGameMode::PlayerDied(ARealmPlayerController* killedPlayer, ARealmPlayerController* playerKiller)
{
	if (!IsValid(killedPlayer) || !IsValid(playerKiller))
		return;

	APlayerCharacter* pc = playerKiller->GetPlayerCharacter();
	if (IsValid(pc))
		pc->ChangeCredits(playerKillWorth);

}