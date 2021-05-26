﻿// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.generated.h"

//----------------------------------------------------------------------------------------------------------------------
/**
*/
UENUM()
enum class ENLActionChangeType : uint8
{
	// No Change
	NONE,
	// Change this action with a new action (this replaces this entry in the stack with a new one)
	CHANGE,
	// Suspend this action for another one
	SUSPEND,
	// This action has completed, resume parent action
	DONE,
};

//----------------------------------------------------------------------------------------------------------------------
/**
 * These are the different event request priorities. They are used 
 */
UENUM(BlueprintType)
enum class ENLEventRequestPriority : uint8
{
	// No priority. Using this as a request priority basically means ignore this request because TRY is the default request priority.
	NONE,
	// Try to accomplish this request
	TRY,
	// Try harder to accomplish this request
	IMPORTANT,
	// Try even harder. If this request has to be thrown out, throw a warning about it.
	CRITICAL,
};

//----------------------------------------------------------------------------------------------------------------------
/**
* An event response structure returned from event implementations
*/
USTRUCT(BlueprintType)
struct FNLEventResponse
{
	GENERATED_BODY()

	FNLEventResponse()
		: ChangeRequest(ENLActionChangeType::NONE)
		, Priority(ENLEventRequestPriority::NONE)
		, Payload(nullptr)
	{}

	FNLEventResponse(ENLActionChangeType changeRequest,
					 ENLEventRequestPriority priority,
					 TSubclassOf<class UNLAction> action,
					 const FString& reason,
					 class UNLActionPayload* payload = nullptr)
		: ChangeRequest(changeRequest)
		, Priority(priority)
		, Action(action)
		, Payload(payload)
		, Reason(reason)
	{}

	// Does this response contain no request?
	FORCEINLINE bool IsNone() const
	{
		return ChangeRequest == ENLActionChangeType::NONE;
	}

	// The request being made
	UPROPERTY()
	ENLActionChangeType ChangeRequest;

	// The priority of the request (so other requests can maybe superseed this request)
	UPROPERTY()
	ENLEventRequestPriority Priority;

	// The action associated with this request (Change and Suspend use this)
	UPROPERTY()
	TSubclassOf<class UNLAction> Action;

	// The payload send with this event
	UPROPERTY()
	class UNLActionPayload* Payload;

	// The reason for this response
	UPROPERTY()
	FString Reason;

	// The name of the event which caused this response
	UPROPERTY()
	FName EventName;
};