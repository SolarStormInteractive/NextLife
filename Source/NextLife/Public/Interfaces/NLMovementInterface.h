// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NavigationSystem\Public\NavFilters\NavigationQueryFilter.h"
#include "NLMovementInterface.generated.h"


UENUM(BlueprintType)
enum class ENLPathFollowingRequestResult : uint8
{
	// Failed outright
	Failed,
	// Already at goal
	AlreadyAtGoal,
	// Request was successful
	RequestSuccessful,
};

USTRUCT(BlueprintType)
struct FNLMoveToGoalRequestResult
{
	GENERATED_BODY()

	FNLMoveToGoalRequestResult(ENLPathFollowingRequestResult resultCode = ENLPathFollowingRequestResult::Failed, int32 requestID = INDEX_NONE)
		: ResultCode(resultCode)
		, RequestID(requestID) {}

	UPROPERTY(BlueprintReadWrite)
	ENLPathFollowingRequestResult ResultCode;

	UPROPERTY(BlueprintReadWrite)
	int32 RequestID;

	operator ENLPathFollowingRequestResult() const { return ResultCode; }
};

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLMovementInterface : public UInterface
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
* Interface to the AI to request movements from the internal NextLife AI system
* If you want NextLife to be able to request moving your AIs from place to place, you need to implement this interface
* how you see fit.
*/
class NEXTLIFE_API INLMovementInterface
{
public:
	GENERATED_BODY()

	// Generalized MoveToGoal call to be implemented in your AI Controller
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|MovementInterface")
	FNLMoveToGoalRequestResult NextLife_MoveToGoal(AActor* Goal, const FVector& Dest, float AcceptanceRadius = -1, bool bUsePathfinding = true,
											        TSubclassOf<UNavigationQueryFilter> FilterClass = nullptr, bool bAllowPartialPath = true);
};
