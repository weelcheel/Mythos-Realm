#include "Realm.h"
#include "RealmEnablerShield.h"

#define LOCTEXT_NAMESPACE "Realm" 

ARealmEnablerShield::ARealmEnablerShield(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	shieldCollision = CreateDefaultSubobject<USphereComponent>(TEXT("shieldCollision"));
	shieldCollision->InitSphereRadius(1150.f);
	shieldCollision->AttachParent = RootComponent;
	shieldCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	shieldCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	shieldCollision->OnComponentBeginOverlap.AddDynamic(this, &ARealmEnablerShield::OnShieldBeginOverlap);
	shieldCollision->OnComponentEndOverlap.AddDynamic(this, &ARealmEnablerShield::OnShieldEndOverlap);
}

void ARealmEnablerShield::OnShieldBeginOverlap(AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyInd, bool bFromSweep, const FHitResult& sweepResult)
{
	//dont run on clients
	if (Role < ROLE_Authority)
		return;

	//debuff any enemies that enter the shield
	AGameCharacter* gc = Cast<AGameCharacter>(otherActor);
	if (IsValid(gc) && gc->IsAlive() && gc->GetTeamIndex() != GetTeamIndex())
	{
		TArray<float> stats;
		TArray<TEnumAsByte<EStat> > statTypes;

		//cut the units defenses in half
		stats.Add(gc->GetCurrentValueForStat(EStat::ES_Def) / -2.f);
		stats.Add(gc->GetCurrentValueForStat(EStat::ES_SpDef) / -2.f);
		statTypes.Add(EStat::ES_Def);
		statTypes.Add(EStat::ES_SpDef);

		gc->AddEffect(LOCTEXT("enablershieldname", "Interference Signal"), LOCTEXT("enablershielddesc", "This unit is in the shield radius of an enemy Enabler Shield. Its Defense and Special Defense are greatly reduced."), statTypes, stats, 0.f, "enablershieldaura");
	}
}

void ARealmEnablerShield::OnShieldEndOverlap(AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyInd)
{
	//dont run on clients
	if (Role < ROLE_Authority)
		return;

	//remove the debuff from any enemies that entered the shield
	AGameCharacter* gc = Cast<AGameCharacter>(otherActor);
	if (IsValid(gc) && gc->IsAlive() && gc->GetTeamIndex() != GetTeamIndex())
		gc->EndEffect("enablershieldaura");
}