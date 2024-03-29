﻿// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"
#include "AIModule\Classes\AITypes.h"
#include "AIModule\Classes\Navigation\PathFollowingComponent.h"
#include "NLMovementEvents.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLMovementEvents : public UInterface
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
* Events related to movement requests
*/
class NEXTLIFE_API INLMovementEvents
{
	GENERATED_BODY()
public:

	/**
	 * Request the AI to move some place
	 * This is only required if your AI needs to stay in sync with movement requests.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|MovementEvents")
	FNLEventResponse Movement_MoveTo(const AActor *goal, const FVector &pos, float range);
	virtual FNLEventResponse Movement_MoveTo_Implementation(const AActor *goal, const FVector &pos, float range) { return FNLEventResponse(); }

	/**
	 * Event letting the AI know a movement was completed
	 */
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|MovementEvents")
	FNLEventResponse Movement_MoveToComplete(FAIRequestID RequestID, const EPathFollowingResult::Type Result);
	virtual FNLEventResponse Movement_MoveToComplete_Implementation(FAIRequestID RequestID, const EPathFollowingResult::Type Result) { return FNLEventResponse(); }
};
