#pragma once

#include "Chat.generated.h"

UENUM()
enum class EChatType : uint8
{
	CT_PlayerChat UMETA(DisplayName = "Player Chat Message"),
	CT_ServerChat UMETA(DisplayName = "Server/Game Chat Message"),
	CT_ChatMax UMETA(Hidden),
};

USTRUCT()
struct FRealmChatEntry
{
	GENERATED_USTRUCT_BODY()

	/* type of message this entry is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	TEnumAsByte<EChatType> chatType;

	/* sender of the message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	FText senderName;

	/* data of the message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	FText chatData;

	/* timestamp of when the server received the chat message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	float timeStamp;

	/* whether or not this chat should be displayed to all members of the game or just the player's team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	bool bAllChat;

	/* team index of the player who made this chat (-1 for no team) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Chat)
	int32 senderTeamIndex;

	FRealmChatEntry()
	{

	}

	FRealmChatEntry(TEnumAsByte<EChatType> ctype, const FText& sender, const FText& data, float time)
	{
		chatType = ctype;
		senderName = sender;
		chatData = data;
		timeStamp = time;
	}
};