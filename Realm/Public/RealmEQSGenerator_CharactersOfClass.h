#pragma once
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "RealmEQSGenerator_CharactersOfClass.generated.h"

class AGameCharacter;

UCLASS(meta = (DisplayName = "Characters of Class"))
class UEnvQueryGenerator_CharacterOfClass : public UEnvQueryGenerator
{
	GENERATED_UCLASS_BODY()

	/** max distance of path between point and context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue SearchRadius;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<AGameCharacter> SearchedActorClass;

	/** context */
	UPROPERTY(EditAnywhere, Category = Generator)
	TSubclassOf<UEnvQueryContext> SearchCenter;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};