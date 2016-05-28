#pragma once
#include "EnvironmentQuery/EnvQueryTest.h"
#include "RealmEQSTest_IsOfTeam.generated.h"

UCLASS(MinimalAPI)
class UEnvQueryTest_IsOfTeam : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	/** whether or not we want the characters to be on this owners team or not be on their team */
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderBoolValue CharactersOnTeam;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};