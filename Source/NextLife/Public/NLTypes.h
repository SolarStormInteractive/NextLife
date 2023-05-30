// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.generated.h"

/**
* Base action payload
*/
UCLASS(Blueprintable)
class NEXTLIFE_API UNLActionPayload : public UObject
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------------
/**
 * The different action changes which can occur, including a NONE which means no change (used to move on)
*/
UENUM(BlueprintType)
enum class ENLActionChangeType : uint8
{
	/** No Change */
	NONE,
	/** Change this action with a new action (this replaces this entry in the stack with a new one) */
	CHANGE,
	/** Suspend this action for another one */
	SUSPEND,
	/** This action has completed, resume parent action */
	DONE,
};

//----------------------------------------------------------------------------------------------------------------------
/**
 * These are the different event request priorities. They are used 
 */
UENUM(BlueprintType)
enum class ENLEventRequestPriority : uint8
{
	/** No priority. Using this as a request priority basically means ignore this request because TRY is the default request priority. */
	NONE,
	/** Try to accomplish this request */
	TRY,
	/** Try harder to accomplish this request */
	IMPORTANT,
	/** Try even harder. If this request has to be thrown out, throw a warning about it. */
	CRITICAL,
};

//----------------------------------------------------------------------------------------------------------------------
/**
* The different types of suspend behaviors
*/
UENUM(BlueprintType)
enum class ENLSuspendBehavior : uint8
{
	/** Normal behavior, suspends from the requesting action which could end the stack above the requesting action. */
	NORMAL,
	/** Append, add to the top of the stack */
	APPEND,
};

//----------------------------------------------------------------------------------------------------------------------
/**
* An event response structure returned from event implementations
* Requests changes to the behaviors action stack
*/
USTRUCT(BlueprintType)
struct FNLEventResponse
{
	GENERATED_BODY()

	FNLEventResponse()
		: ChangeRequest(ENLActionChangeType::NONE)
		, Priority(ENLEventRequestPriority::NONE)
		, Payload(nullptr)
		, SuspendBehavior(ENLSuspendBehavior::NORMAL)
	{}

	FNLEventResponse(ENLActionChangeType changeRequest,
					 ENLEventRequestPriority priority,
					 TSubclassOf<class UNLAction> action,
					 const FString& reason,
					 class UNLActionPayload* payload = nullptr,
					 const ENLSuspendBehavior suspendBehavior = ENLSuspendBehavior::NORMAL)
		: ChangeRequest(changeRequest)
		, Priority(priority)
		, Action(action)
		, Payload(payload)
		, Reason(reason)
		, SuspendBehavior(suspendBehavior)
	{}

	// Does this response contain no request?
	FORCEINLINE bool IsNone() const
	{
		return ChangeRequest == ENLActionChangeType::NONE;
	}

	// True if this event does not cause destruction to the stack (appends only, no ends)
	FORCEINLINE bool IsNonDestructive(const bool hasNoNextAction) const
	{
		if(ChangeRequest != ENLActionChangeType::SUSPEND)
		{
			return false;
		}

		if(SuspendBehavior == ENLSuspendBehavior::APPEND)
		{
			return true;
		}
		
		return hasNoNextAction;
	}

	// The request being made
	UPROPERTY(SaveGame)
	ENLActionChangeType ChangeRequest;

	// The priority of the request (so other requests can maybe superseed this request)
	UPROPERTY(SaveGame)
	ENLEventRequestPriority Priority;

	// The action associated with this request (Change and Suspend use this)
	UPROPERTY(SaveGame)
	TSubclassOf<class UNLAction> Action;

	// The payload send with this event
	UPROPERTY(SaveGame)
	class UNLActionPayload* Payload;

	// The reason for this response
	UPROPERTY(SaveGame)
	FString Reason;

	// The name of the event which caused this response
	UPROPERTY(SaveGame)
	FName EventName;

	// The behavior used when suspending
	// This is only used when ChangeRequest is SUSPEND
	// See ENLSuspendBehavior for details.
	UPROPERTY(SaveGame)
	ENLSuspendBehavior SuspendBehavior;
};
