#include "Realm.h"
#include "RealmPlayerController.h"
#include "RealmMoveController.h"
#include "PlayerCharacter.h"
#include "SpectatorCharacter.h"
#include "UnrealNetwork.h"
#include "PlayerHUD.h"
#include "Mod.h"
#include "RealmPlayerState.h"
#include "RealmGameMode.h"
#include "RealmGameInstance.h"
#include "MinimapActor.h"
#include "RealmFogOfWarManager.h"

ARealmPlayerController::ARealmPlayerController(const FObjectInitializer& objectInitializer)
:Super(objectInitializer)
{
	bShowMouseCursor = true;

	//debug code
	//static ConstructorHelpers::FClassFinder<APlayerCharacter> PlayerPawnBPClass(TEXT("/Game/Realm/Characters/PCs/Leighton/Leighton"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
		//defaultCharacterClass = PlayerPawnBPClass.Class;
	//}
}

void ARealmPlayerController::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ACameraActor> camitr(GetWorld()); camitr; ++camitr)
	{
		ACameraActor* ca = (*camitr);
		if (ca->Tags.Contains("startCam"))
			SetViewTarget((*camitr));
	}
}

void ARealmPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("IngameStore", IE_Pressed, this, &ARealmPlayerController::OnToggleIngameStore);
	InputComponent->BindAction("TargetClick", IE_Pressed, this, &ARealmPlayerController::OnTargetSelect);
	InputComponent->BindAction("BaseTeleport", IE_Pressed, this, &ARealmPlayerController::OnBaseTeleport);
}

void ARealmPlayerController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	//@todo: get the character class the player chose to play with
	ASpectatorCharacter* sc = Cast<ASpectatorCharacter>(aPawn);
	if (!IsValid(moveController))
	{
		moveController = GetWorld()->SpawnActor<ARealmMoveController>(FVector::ZeroVector, FRotator::ZeroRotator);
		moveController->SetOwner(this);

		if (sc)
		{
			//sc->SetPlayerController(this);
			//sc->GetCharacterMovement()->SetMovementMode(MOVE_Flying);;
			//sc->GetCharacterMovement()->MaxFlySpeed = 600.f;
			SetViewTarget(sc);
			//ClientSetRTSCameraViewTarget(sc);
		}

		if (!playerCharacter)
		{
			APlayerCharacter* pc = GetWorld()->SpawnActor<APlayerCharacter>(GetDefaultCharacterClass(), aPawn->GetActorLocation(), aPawn->GetActorRotation());
			ARealmPlayerState* ps = Cast<ARealmPlayerState>(PlayerState);

			if (pc)
			{
				moveController->Possess(pc);
				playerCharacter = pc;
				playerCharacter->SetPlayerController(this);
				playerCharacter->SetOwner(this);
				playerCharacter->ChangeCredits(GetWorld()->GetAuthGameMode<ARealmGameMode>()->GetStartingCredits());

				playerCharacter->skillPoints = 1;
				for (int32 i = 0; i < GetWorld()->GetAuthGameMode<ARealmGameMode>()->startingSkillPoints-1; i++)
					playerCharacter->LevelUp();

				playerCharacter->GiveCharacterExperience(FMath::Square(GetWorld()->GetAuthGameMode<ARealmGameMode>()->startingSkillPoints) / FMath::Square(EXP_CONST));
				playerCharacter->OnCharacterSpawned();

				if (ps)
				{
					playerCharacter->PlayerState = ps;
					playerCharacter->SetTeamIndex(ps->GetTeamIndex());

					/*FString fogName = GetFName().ToString() + ".fogManager";
					fogOfWar = NewObject<URealmFogofWarManager>(this, FName(*fogName));
					fogOfWar->teamIndex = ps->GetTeamIndex();
					fogOfWar->playerOwner = this;
					fogOfWar->StartCalculatingVisibility();*/
				}
			}
		}
	}
}

void ARealmPlayerController::ClientSetRTSCameraViewTarget_Implementation(ASpectatorCharacter* scharacter)
{
	
}

bool ARealmPlayerController::ServerMoveCommand_Validate(FVector_NetQuantize targetLocation)
{
	return true;
}

void ARealmPlayerController::ServerMoveCommand_Implementation(FVector_NetQuantize targetLocation)
{
	if (!IsValid(this) || !IsValid(playerCharacter))
		return;

	if (!playerCharacter->CanMove())
		return;
	
	ServerClearAttackCommands();

	if (GetWorldTimerManager().GetTimerRemaining(playerCharacter->baseTeleportTimer) > 0.f)
		ServerStopBaseTeleport();

	if (IsValid(moveController))
	{
		moveController->MoveToLocation(targetLocation);
		FRotator newDir = (targetLocation - playerCharacter->GetActorLocation()).Rotation();
		newDir.Pitch = 0.f;

		playerCharacter->SetActorRotation(FMath::RInterpTo(playerCharacter->GetActorRotation(), newDir, GetWorld()->DeltaTimeSeconds, 5.f));
		//playerCharacter->SetActorRotation(newDir);
	}
}

bool ARealmPlayerController::ServerProcessSkillInputData_Validate(const FHitResult& targetData)
{
	return true;
}

void ARealmPlayerController::ServerProcessSkillInputData_Implementation(const FHitResult& targetData)
{
	TArray<ASkill*> skills;
	playerCharacter->GetSkillManager()->GetSkills(skills);
	for (ASkill* skill : skills)
	{
		if (skill->GetSkillState() == ESkillState::Performing)
			skill->TargetInputReceivedWhilePerforming(targetData);
	}
}

bool ARealmPlayerController::ServerStartAutoAttack_Validate(AGameCharacter* target)
{
	return true;
}

void ARealmPlayerController::ServerStartAutoAttack_Implementation(AGameCharacter* target)
{
	if (!IsValid(playerCharacter))
		return;

	if (playerCharacter->GetCurrentTarget() == target)
		return;

	if (!playerCharacter->CanAutoAttack())
		return;

	ServerClearMoveCommands();
	ServerStopBaseTeleport();

	playerCharacter->SetCurrentTarget(target);

	if (playerCharacter->CanMove())
		GetPlayerInAutoAttackRange();
}

bool ARealmPlayerController::ServerClearMoveCommands_Validate()
{
	return true;
}

void ARealmPlayerController::ServerClearMoveCommands_Implementation()
{
	if (!IsValid(moveController))
		return;

	moveController->StopMovement();
}

bool ARealmPlayerController::ServerClearAttackCommands_Validate()
{
	return true;
}

void ARealmPlayerController::ServerClearAttackCommands_Implementation()
{
	if (!IsValid(playerCharacter))
		return;

	playerCharacter->SetCurrentTarget(nullptr);
	GetWorldTimerManager().ClearTimer(aaRangeTimer);
	playerCharacter->StopAutoAttack();
}

bool ARealmPlayerController::ServerUseSkill_Validate(int32 index, const FHitResult& hitInfo)
{
	return true;
}

void ARealmPlayerController::ServerUseSkill_Implementation(int32 index, const FHitResult& hitInfo)
{
	if (!IsValid(playerCharacter))
		return;

	AGameCharacter* gc = Cast<AGameCharacter>(hitInfo.GetActor());

	if (playerCharacter->CanPerformSkills())
	{
		ServerStopBaseTeleport();
		playerCharacter->UseSkill(index, hitInfo.ImpactPoint, gc);
	}
}

bool ARealmPlayerController::ServerUseMod_Validate(int32 index, FHitResult const& hit)
{
	return true;
}

void ARealmPlayerController::ServerUseMod_Implementation(int32 index, FHitResult const& hit)
{
	if (!IsValid(playerCharacter))
		return;

	playerCharacter->UseMod(index, hit);
}

bool ARealmPlayerController::ServerChooseCharacter_Validate(TSubclassOf<APlayerCharacter> chosenCharacter)
{
	return chosenCharacter != nullptr;
}

void ARealmPlayerController::ServerChooseCharacter_Implementation(TSubclassOf<APlayerCharacter> chosenCharacter)
{
	GetWorld()->GetAuthGameMode<ARealmGameMode>()->PlayerSelectedCharacter(this, chosenCharacter);
}

void ARealmPlayerController::GetPlayerInAutoAttackRange()
{
	if (!IsValid(playerCharacter) || !IsValid(playerCharacter->GetCurrentTarget()) || !IsValid(moveController))
		return;

	if (!playerCharacter->GetCurrentTarget()->IsAlive() || !playerCharacter->CanSeeOtherCharacter(playerCharacter->GetCurrentTarget()))
	{
		ServerClearAttackCommands();
		ServerClearMoveCommands();
		return;
	}

	float distanceSq = (playerCharacter->GetActorLocation() - playerCharacter->GetCurrentTarget()->GetActorLocation()).SizeSquared2D();
	float aaRange = FMath::Square(playerCharacter->GetStatsManager()->GetCurrentValueForStat(EStat::ES_AARange));
	if (distanceSq <= aaRange)
		playerCharacter->StartAutoAttack();
	else
	{
		moveController->MoveToActor(playerCharacter->GetCurrentTarget());
		GetWorldTimerManager().SetTimer(aaRangeTimer, this, &ARealmPlayerController::GetPlayerInAutoAttackRange, 0.05f);
	}
}

TSubclassOf<APlayerCharacter> ARealmPlayerController::GetDefaultCharacterClass() const
{
	ARealmPlayerState* ps = Cast<ARealmPlayerState>(PlayerState);
	if (IsValid(ps) && IsValid(ps->GetChosenCharacterClass()))
		return ps->GetChosenCharacterClass();
	
	return nullptr;
}

APlayerCharacter* ARealmPlayerController::GetPlayerCharacter() const
{
	return playerCharacter;
}

void ARealmPlayerController::ClientInitIngameStore_Implementation(const TArray<TSubclassOf<AMod> >& modStore)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->InitIngameStore(modStore);
}

void ARealmPlayerController::ClientOpenPregameScreen_Implementation()
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->OpenPregameScreen();
}

void ARealmPlayerController::ClientOpenCharacterSelectScreen_Implementation(const TArray<TSubclassOf<APlayerCharacter> >& availableCharacters)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->OpenCharacterSelectScreen(availableCharacters);
}

void ARealmPlayerController::ClientOpenPlayerHUD_Implementation()
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->OpenPlayerHUD();
}

void ARealmPlayerController::OnOpenIngameStore()
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->OpenInGameStore();
}

void ARealmPlayerController::OnCloseIngameStore()
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->CloseInGameStore();
}

void ARealmPlayerController::OnToggleIngameStore()
{
	bIngameStoreOpen = !bIngameStoreOpen;
	if (bIngameStoreOpen)
		OnOpenIngameStore();
	else
		OnCloseIngameStore();
}

void ARealmPlayerController::OnTargetSelect()
{
	FHitResult hit;

	if (SelectUnitUnderMouse(ECC_Visibility, true, hit))
	{
		AGameCharacter* gc = Cast<AGameCharacter>(hit.GetActor());
		if (IsValid(gc))
		{
			if (gc->IsAlive())
				infoTarget = gc;
			else
				infoTarget = nullptr;
		}
		else
			infoTarget = nullptr;
	}
	else
		infoTarget = nullptr;
}

bool ARealmPlayerController::ServerBuyPlayerMod_Validate(TSubclassOf<AMod> wantedMod)
{
	return wantedMod != nullptr;
}

void ARealmPlayerController::ServerBuyPlayerMod_Implementation(TSubclassOf<AMod> wantedMod)
{
	AMod* modToBuy = Cast<AMod>(wantedMod->GetDefaultObject());
	if (modToBuy)
	{
		AMod* modToAdd;
		if (modToBuy->CanCharacterBuyThisMod(GetPlayerCharacter()))
		{
			modToAdd = GetWorld()->SpawnActor<AMod>(wantedMod, GetPlayerCharacter()->GetActorLocation(), GetPlayerCharacter()->GetActorRotation());
			if (IsValid(modToAdd))
			{
				modToAdd->SetCharacterOwner(GetPlayerCharacter());
				modToAdd->CharacterPurchasedMod(GetPlayerCharacter());
				GetPlayerCharacter()->AddMod(modToAdd);
			}
		}
	}
}

bool ARealmPlayerController::ServerSellPlayerMod_Validate(int32 index)
{
	return true;
}

void ARealmPlayerController::ServerSellPlayerMod_Implementation(int32 index)
{
	APlayerCharacter* pc = GetPlayerCharacter();
	if (!IsValid(pc) || index < 0 || index > 6)
		return;

	if (index < pc->GetModCount())
	{
		AMod* modToSell = pc->GetMods()[index];
		if (IsValid(modToSell))
			pc->ChangeCredits(modToSell->GetCost() / 2.f, pc->GetActorLocation());
	}

	pc->RemoveMod(index);
}

ARealmMoveController* ARealmPlayerController::GetMoveController() const
{
	return moveController;
}

bool ARealmPlayerController::GetUnitsUnderMouse(ECollisionChannel TraceChannel, bool bTraceComplex, TArray<FHitResult>& hits) const
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			FVector WorldOrigin;
			FVector WorldDirection;
			if (UGameplayStatics::DeprojectScreenToWorld(this, MousePosition, WorldOrigin, WorldDirection) == true)
			{
				//return GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance, TraceChannel, CollisionQueryParams);
				//return a sweep so we don't have to be so precise with the mouse pointer
				return GetWorld()->SweepMultiByChannel(hits, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance, IsValid(playerCharacter) ? playerCharacter->GetActorRotation().Quaternion() : FRotator::ZeroRotator.Quaternion(), TraceChannel, FCollisionShape::MakeSphere(22.5f));
			}
		}
	}

	return false;
}

bool ARealmPlayerController::SelectUnitUnderMouse(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& chosenHit) const
{
	if (!IsValid(GetPlayerCharacter()))
		return false;

	TArray<FHitResult> hits;
	if (GetUnitsUnderMouse(ECC_Camera, true, hits))
	{
		FHitResult friendlyUnit, otherUnit;

		for (FHitResult testHit : hits)
		{
			AGameCharacter* gc = Cast<AGameCharacter>(testHit.GetActor());
			if (!IsValid(gc))
				continue;

			if (!gc->IsTargetable())
				continue;

			if (gc->GetTeamIndex() != GetPlayerCharacter()->GetTeamIndex())
			{
				chosenHit = testHit;
				return true;
			}
			else if (gc != GetPlayerCharacter())
				friendlyUnit = testHit;
			else
				otherUnit = testHit;
		}

		//select order: 1) enemy units 2) friendly units 3)self
		if (IsValid(friendlyUnit.GetComponent()))
			chosenHit = friendlyUnit;
		else
			chosenHit = hits[hits.Num() - 1];

		return true;
	}

	return false;
}

void ARealmPlayerController::OnDeathMessage(ARealmPlayerState* killer, ARealmPlayerState* killed, APawn* killerPawn)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->NotifyPlayerKill(killer, killed, killerPawn, killed == PlayerState, killer == PlayerState);
}

void ARealmPlayerController::OnObjectiveDeathMessage(APawn* killerPawn, ARealmObjective* objectiveDestroyed)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->NotifyObjectiveKill(killerPawn, objectiveDestroyed);
}

void ARealmPlayerController::ClientSendEndgameUserID_Implementation()
{
	URealmGameInstance* instance = Cast<URealmGameInstance>(GetGameInstance());
	if (instance)
		ServerReceiveEndgameUserID(instance->GetUserID());
}

void ARealmPlayerController::ClientOpenEndgameUI_Implementation(int32 winningTeam)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->NotifyEndGame(winningTeam);
}

bool ARealmPlayerController::ServerReceiveEndgameUserID_Validate(const FString& userid)
{
	return userid != "";
}

void ARealmPlayerController::ServerReceiveEndgameUserID_Implementation(const FString& userid)
{
	ARealmPlayerState* ps = Cast<ARealmPlayerState>(PlayerState);
	int32 teamInd = 0;
	if (IsValid(ps))
		teamInd = ps->GetTeamIndex();

	if (GetWorld()->GetAuthGameMode<ARealmGameMode>())
		GetWorld()->GetAuthGameMode<ARealmGameMode>()->ReceiveEndgameStats(userid, teamInd);
}

bool ARealmPlayerController::ServerOnUpgradeSkill_Validate(int32 index)
{
	return true;
}

void ARealmPlayerController::ServerOnUpgradeSkill_Implementation(int32 index)
{
	if (IsValid(GetPlayerCharacter()))
		GetPlayerCharacter()->OnUpgradeSkill(index);
}

void ARealmPlayerController::ClientShowCreditGain_Implementation(const FVector& worldLoc, int32 creditAmt)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->ShowCreditGain(worldLoc, creditAmt);
}

bool ARealmPlayerController::ServerLockPlayerCamera_Validate(AActor* newFocus)
{
	return true;
}

bool ARealmPlayerController::ServerUnlockPlayerCamera_Validate()
{
	return true;
}

void ARealmPlayerController::ServerLockPlayerCamera_Implementation(AActor* newFocus)
{
	ASpectatorCharacter* sc = Cast<ASpectatorCharacter>(GetCharacter());
	if (!IsValid(sc))
		return;

	sc->cameraLockTarget = newFocus;
}

void ARealmPlayerController::ServerUnlockPlayerCamera_Implementation()
{
	ASpectatorCharacter* sc = Cast<ASpectatorCharacter>(GetCharacter());
	if (!IsValid(sc))
		return;

	if (IsValid(sc->cameraLockTarget))
		sc->SetActorLocation(sc->cameraLockTarget->GetActorLocation());

	sc->cameraLockTarget = nullptr;
	sc->bLockedOnCharacter = false;
}

void ARealmPlayerController::OnBaseTeleport()
{
	ServerStartBaseTeleport();
}

void ARealmPlayerController::StartBaseTeleport(bool bStarting)
{
	if (!IsValid(playerCharacter))
		return;

	if (bStarting)
		playerCharacter->StartBaseTeleport();
	else
		playerCharacter->StopBaseTeleport();
}

bool ARealmPlayerController::ServerStartBaseTeleport_Validate()
{
	return true;
}

void ARealmPlayerController::ServerStartBaseTeleport_Implementation()
{
	StartBaseTeleport(true);
}

bool ARealmPlayerController::ServerStopBaseTeleport_Validate()
{
	return true;
}

void ARealmPlayerController::ServerStopBaseTeleport_Implementation()
{
	StartBaseTeleport(false);
}

void ARealmPlayerController::ClientReceiveChat(const FRealmChatEntry& incomingChat)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->PlayerReceiveChat(incomingChat);
}

void ARealmPlayerController::ClientToggleChat()
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->PlayerToogleChat();
}

bool ARealmPlayerController::ServerReceiveChat_Validate(const FRealmChatEntry& broadcastChat)
{
	return true;
}

void ARealmPlayerController::ServerReceiveChat_Implementation(const FRealmChatEntry& broadcastChat)
{
	ARealmGameState* gs = Cast<ARealmGameState>(GetWorld()->GetGameState());
	if (IsValid(gs))
		gs->BroadcastChat(broadcastChat);
}

void ARealmPlayerController::CharacterChosenForGame_Implementation(ARealmPlayerState* choosingPlayer, TSubclassOf<APlayerCharacter> chosenCharacter)
{
	APlayerHUD* hud = Cast<APlayerHUD>(GetHUD());
	if (IsValid(hud))
		hud->CharacterSelected(choosingPlayer, chosenCharacter);
}

void ARealmPlayerController::GameEnded()
{
	
}

void ARealmPlayerController::OnRep_SightList()
{
	if (!IsValid(this) || !IsValidLowLevelFast())
		return;

	for (TActorIterator<AGameCharacter> chr(GetWorld()); chr; ++chr)
	{
		AGameCharacter* gc = *chr;
		if (!IsValid(gc))
			continue;

		gc->SetActorHiddenInGame(!sightList.Contains(gc));
		TArray<AActor*> attached;
		gc->GetAttachedActors(attached);

		for (AActor* attachee : attached)
			attachee->SetActorHiddenInGame(!sightList.Contains(gc));
	}
}

void ARealmPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealmPlayerController, playerCharacter);
	DOREPLIFETIME(ARealmPlayerController, sightList);
	//DOREPLIFETIME(ARealmPlayerController, fogOfWar);
}