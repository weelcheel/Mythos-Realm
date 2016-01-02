#pragma once

#include "RankDivisions.generated.h"

UENUM(BlueprintType)
enum class EPlayerLeague : uint8
{
	PL_Bronze UMETA(DisplayName = "Bronze League"),
	PL_Silver UMETA(DisplayName = "Silver League"),
	PL_Gold UMETA(DisplayName = "Gold League"),
	PL_Diamond UMETA(DisplayName = "Diamond League"),
	PL_Obsidian UMETA(DisplayName = "Obsidian League"),
	PL_Mythos UMETA(DisplayName = "Mythos League"),
	PL_Legend UMETA(DisplayName = "Legends"),
	PL_Max UMETA(Hidden)
};