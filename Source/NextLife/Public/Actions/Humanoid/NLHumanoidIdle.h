// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLAction.h"
#include "EventSets/NLSensingEvents.h"
#include "NLHumanoidIdle.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * Simple Humanoid Idle
 */
UCLASS()
class NEXTLIFE_API UNLHumanoidIdle : public UNLAction
								 , public INLSensingEvents
{
	GENERATED_BODY()

	// Sensing Events
	virtual FNLEventResponse Sense_Sight_Implementation(APawn* subject, bool indirect = false) override;
	virtual FNLEventResponse Sense_SightLost_Implementation(APawn* subject) override;
};
