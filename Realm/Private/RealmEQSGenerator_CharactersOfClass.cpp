#include "Realm.h"
#include "RealmEQSGenerator_CharactersOfClass.h"
#include "GameCharacter.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

#define LOCTEXT_NAMESPACE "RealmEnvQueryGenerator"

UEnvQueryGenerator_CharacterOfClass::UEnvQueryGenerator_CharacterOfClass(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SearchCenter = UEnvQueryContext_Querier::StaticClass();
	ItemType = UEnvQueryItemType_Actor::StaticClass();
	SearchRadius.DefaultValue = 500.0f;
	SearchedActorClass = AGameCharacter::StaticClass();
}

void UEnvQueryGenerator_CharacterOfClass::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	SearchRadius.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	float RadiusValue = SearchRadius.GetValue();

	UWorld* World = GEngine->GetWorldFromContextObject(QueryInstance.Owner.Get());
	AGameCharacter* actor = Cast<AGameCharacter>(QueryInstance.Owner.Get());

	if (World == NULL || SearchedActorClass == NULL || !IsValid(actor))
	{
		return;
	}

	const float RadiusSq = FMath::Square(RadiusValue);

	for (TActorIterator<AGameCharacter> ItActor = TActorIterator<AGameCharacter>(World, SearchedActorClass); ItActor; ++ItActor)
	{
		if (FVector::DistSquared(actor->GetActorLocation(), ItActor->GetActorLocation()) < RadiusSq && actor != *ItActor && actor->GetTeamIndex() != (*ItActor)->GetTeamIndex() && (*ItActor)->IsAlive())
			QueryInstance.AddItemData<UEnvQueryItemType_Actor>(*ItActor);
	}
}

FText UEnvQueryGenerator_CharacterOfClass::GetDescriptionTitle() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DescriptionTitle"), Super::GetDescriptionTitle());
	Args.Add(TEXT("ActorsClass"), FText::FromString(GetNameSafe(SearchedActorClass)));
	Args.Add(TEXT("DescribeContext"), UEnvQueryTypes::DescribeContext(SearchCenter));

	return FText::Format(LOCTEXT("DescriptionGenerateActorsAroundContext", "{DescriptionTitle}: generate set of enemy characters of {ActorsClass} around {DescribeContext}"), Args);
};

FText UEnvQueryGenerator_CharacterOfClass::GetDescriptionDetails() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Radius"), FText::FromString(SearchRadius.ToString()));

	FText Desc = FText::Format(LOCTEXT("ActorsOfClassDescription", "radius: {Radius}"), Args);

	return Desc;
}

#undef LOCTEXT_NAMESPACE