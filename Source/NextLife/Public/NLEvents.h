// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLEvents.generated.h"

//----------------------------------------------------------------------------------------------------------------------
/**
 * An event causes a request, these are the different types
 */
UENUM()
enum class ENLEventRequest : uint8
{
	// Nothing to request
	NONE,
	// Request to change this action to a new action (this replaces this entry in the stack with a new one)
	CHANGE,
	// Request to suspend this action for another one (Burried actions can suspend child actions to start a new action)
	SUSPEND,
	// Request the action be done (Applies only to the action receiving the event)
	DONE,
	// 
	TAKE_OVER,
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
		: Request(ENLEventRequest::NONE)
		, Priority(ENLEventRequestPriority::NONE)
		, Payload(nullptr)
	{}

	FNLEventResponse(ENLEventRequest request,
					 ENLEventRequestPriority priority,
					 TSubclassOf<class UNLAction> action,
					 const FString& reason,
					 class UNLActionPayload* payload = nullptr)
		: Request(request)
		, Priority(priority)
		, Action(action)
		, Payload(payload)
		, Reason(reason)
	{}

	// The request being made
	UPROPERTY()
	ENLEventRequest Request;

	// The priority of the request (so other requests can maybe superseed this request)
	UPROPERTY()
	ENLEventRequestPriority Priority;

	// The action associated with this request
	UPROPERTY()
	TSubclassOf<class UNLAction> Action;

	// The payload send with this event
	UPROPERTY()
	class UNLActionPayload* Payload;

	// The reason for this response
	UPROPERTY()
	FString Reason;
};
