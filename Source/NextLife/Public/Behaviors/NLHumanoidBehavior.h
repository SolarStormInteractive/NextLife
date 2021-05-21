// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLBehavior.h"

// Event Sets
#include "EventSets/NLSensingEvents.h"
#include "EventSets/NLInflictionEvents.h"

#include "NLHumanoidBehavior.generated.h"

/**
 * A basic humanoid behavior that can sense using eyes, sound, and touch
 */
UCLASS()
class NEXTLIFE_API UNLHumanoidBehavior : public UNLBehavior
{
	GENERATED_BODY()
};
