// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"
#include "NLSensingEvents.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLSensingEvents : public UInterface
{
    GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
 * General Events related to sensing
*/
class NEXTLIFE_API INLSensingEvents
{
public:
    GENERATED_BODY()

	/** On sight of a pawn
	 * @param subject - The noticed pawn subject
	 * @param indirect - True if this sight was through another eyes, ex : squad member reports an enemy
	 */
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
	FNLEventResponse Sense_Sight(APawn* subject, bool indirect = false);
	virtual FNLEventResponse Sense_Sight_Implementation(APawn* subject, bool indirect) { return FNLEventResponse(); };

	// On lost sight of a pawn (only pawns we see)
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
	FNLEventResponse Sense_SightLost(APawn* subject);
	virtual FNLEventResponse Sense_SightLost_Implementation(APawn* subject) { return FNLEventResponse(); };

	// On hearing an interesting sound we care about
	// flags allows sending some extra information about the sound if needed. Align this to an enum if needed.
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
	FNLEventResponse Sense_Sound(APawn *OtherActor, const FVector &Location, float Volume, int32 flags = 0);
	virtual FNLEventResponse Sense_Sound_Implementation(APawn *OtherActor, const FVector &Location, float Volume, int32 flags) { return FNLEventResponse(); }

    // On contact with a world surface or actor
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
    FNLEventResponse Sense_Contact(AActor* other, const FHitResult& hitResult);
    virtual FNLEventResponse Sense_Contact_Implementation(AActor* other, const FHitResult& hitResult) { return FNLEventResponse(); }
};
