// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"
#include "NLSquadEvents.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class UNLSquadEvents : public UInterface
{
    GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
 * General Events related to squad AI being commanded
 * This is included as a sort of 'Hint' to the general commands a squad member usually recieves.
*/
class INLSquadEvents
{
public:
    GENERATED_BODY()

    // Commands this AI to get to a cover location close to them
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SquadCommandEvents")
    FNLEventResponse SquadCommand_GetToCover(const FVector& coverLocation);
    virtual FNLEventResponse SquadCommand_GetToCover_Implementation(const FVector& coverLocation) { return FNLEventResponse(); }

    // Commands this AI to get to a cover location closer to the player
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SquadCommandEvents")
    FNLEventResponse SquadCommand_AdvanceCover(const FVector& coverLocation);
    virtual FNLEventResponse SquadCommand_AdvanceCover_Implementation(const FVector& coverLocation) { return FNLEventResponse(); }

    // Commands this AI to move along firing lines (the vector of attack) to push the player
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SquadCommandEvents")
    FNLEventResponse SquadCommand_OrderlyAdvance(const FVector& firingLocation, const FVector& firingLineDirection);
    virtual FNLEventResponse SquadCommand_OrderlyAdvance_Implementation(const FVector& firingLocation, const FVector& firingLineDirection) { return FNLEventResponse(); }

    // Commands this AI to search for the player in a location
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SquadCommandEvents")
    FNLEventResponse SquadCommand_Search(const FVector& searchLocation);
    virtual FNLEventResponse SquadCommand_Search_Implementation(const FVector& searchLocation) { return FNLEventResponse(); }
};
