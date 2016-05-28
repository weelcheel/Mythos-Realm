#include "Realm.h"
#include "RealmEQSTest_IsOfTeam.h"
#include "GameCharacter.h"

#define LOCTEXT_NAMESPACE "RealmEnvQueryTest"

UEnvQueryTest_IsOfTeam::UEnvQueryTest_IsOfTeam(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UEnvQueryTest_IsOfTeam::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* DataOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(DataOwner, QueryInstance.QueryID);

	bool bWantsHit = BoolValue.GetValue();

	AController* botController = Cast<AController>(DataOwner);
	if (!IsValid(botController))
		return;

	AGameCharacter* gc = Cast<AGameCharacter>(botController->GetPawn());
	if (!IsValid(gc))
		return;

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AGameCharacter* ItemActor = (AGameCharacter*)GetItemActor(QueryInstance, It.GetIndex());
		if (!IsValid(ItemActor))
			continue;

		const bool bIsOnTeam = gc->GetTeamIndex() == ItemActor->GetTeamIndex();
		It.SetScore(TestPurpose, FilterType, bIsOnTeam, bWantsHit);
	}
}

FText UEnvQueryTest_IsOfTeam::GetDescriptionTitle() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DescriptionTitle"), Super::GetDescriptionTitle());

	return FText::Format(LOCTEXT("TitleIsOfTeam", "{DescriptionTitle}: test whether or not the characters are on the same team."), Args);
}

FText UEnvQueryTest_IsOfTeam::GetDescriptionDetails() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DescriptionTitle"), GetDescriptionTitle());

	return FText::Format(LOCTEXT("DescriptionIsOfTeam", "{DescriptionTitle}: same team checks."), Args);
}

#undef LOCTEXT_NAMESPACE