// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLEvents.h"

#include "NLAction.generated.h"

//----------------------------------------------------------------------------------------------------------------------
/**
 * An event causes a request, these are the different types
 */
UENUM()
enum class ENLActionResultType : uint8
{
	// Continue running this action next time
	CONTINUE,
	// Change this action with a new action (this replaces this entry in the stack with a new one)
	CHANGE,
	// Suspend this action for another one
	SUSPEND,
	// This action has completed, resume parent action
	DONE,
};

//----------------------------------------------------------------------------------------------------------------------
/**
* An action result structure returned from action Start and Update
*/
USTRUCT(BlueprintType)
struct FNLActionResult
{
	GENERATED_BODY()

	FNLActionResult()
		: Result(ENLActionResultType::CONTINUE)
		, Payload(nullptr)
	{}

	FNLActionResult(ENLActionResultType result,
					TSubclassOf<class UNLAction> action,
					const FString& reason,
					class UNLActionPayload* payload = nullptr)
		: Result(result)
		, Action(action)
		, Payload(payload)
		, Reason(reason)
	{}

	// The request being made
	UPROPERTY()
	ENLActionResultType Result;

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

/**
 * Base action payload
 */
UCLASS(Blueprintable)
class NEXTLIFE_API UNLActionPayload : public UObject
{
	GENERATED_BODY()
public:
	
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Action : An action is something the AI should do to complete a task
 *               A Behavior starts an initial action and other actions can start from actions creating an action list.
*/
UCLASS(Blueprintable)
class NEXTLIFE_API UNLAction : public UObject
{
    GENERATED_BODY()
public:
    UNLAction();
	
	// The type of payload expected by this action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NextLife|Action")
	TSubclassOf<UNLActionPayload> PayloadClass;

	// Gets the pawn which is being controlled by the AI controller which is running NextLife as the AI brain.
	// If you are getting the pawn owner to cast it to a specific class to get information, perhaps consider using a blackboard instead.
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class APawn* GetPawnOwner() const;

	// Gets the AI controller which is running NextLife as the AI brain.
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class AAIController* GetAIOwner() const;

	// Get the behavior this action is a part of
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class UNLBehavior* GetBehavior() const;

	// Gets the currently assigned blackboard component (if one has been assigned in AIController via UseBlackboard)
	// Blackboards can be useful for storing information in a generic fassion for the AI to use while executing.
	// Passing information to an AI through a blackboard can generalize your AI routines to be usable by many different pawn types.
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class UBlackboardComponent* GetBlackboard() const;

	//-----------------------------------------------------------------------------------------
	/**
	 * Start the action, the result will be immediately processed which could cause an immediate transition to another action.
	 * If a transition occurs, those new actions will follow the same rule of Start and immediate processing.
	 * Any started actions will end in an OnEnd call. If suspended, also expect a OnSuspend call.
	 * When an action is resumed from another action, expect a OnResume call.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	FNLActionResult OnStart(const UNLAction *priorAction);
	virtual FNLActionResult OnStart_Implementation(const UNLAction* priorAction)
	{
		return Continue();
	}

	/**
	* Do the work of the Action. It is possible for Update to not be
	* called between a given OnStart/OnEnd pair due to immediate transitions.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	FNLActionResult OnUpdate(const float deltaSeconds);
	virtual FNLActionResult OnUpdate_Implementation(const float deltaSeconds)
	{
		return Continue();
	}

	/**
	 * Invoked when an Action is ended for any reason
	 * Any state changes made to your AIs should be reverted during this call.
	 * If all actions in the action stack were to end, idealy your AI should be left in a clean state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	void OnEnd(const UNLAction *nextAction);
	virtual void OnEnd_Implementation(const UNLAction *nextAction)
	{
	}

	/*
	* When an Action is suspended by a new action.
	* Return true if this is ok. Return false if this action should instead be replaced.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	bool OnSuspend(const UNLAction *interruptingAction);
	virtual bool OnSuspend_Implementation(const UNLAction *interruptingAction)
	{
		return true;
	}

	// When an Action is resumed after being suspended
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	FNLActionResult OnResume(const UNLAction *resumedFromAction);
	virtual FNLActionResult OnResume_Implementation(const UNLAction *resumedFromAction)
	{
		return Continue();
	}

	// Continue the action
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	static FNLActionResult Continue()
	{
		return FNLActionResult();
	}

	// change to another action
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	static FNLActionResult ChangeTo(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionResultType::CHANGE, action, reason, payload);
	}

	// suspension to another action
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	static FNLActionResult SuspendFor(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionResultType::SUSPEND, action, reason, payload);
	}

	// the action is done
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	static FNLActionResult Done(TSubclassOf<class UNLAction> action, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionResultType::DONE, action, reason);
	}

	// Return response to continue (no request being made, let the parent actions handle this event)
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	static FNLEventResponse TryContinue()
	{
		return FNLEventResponse();
	}

	// Return response to request a change to another action
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	static FNLEventResponse TryChangeTo(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::CHANGE, priority, action, reason, payload);
	}

	// Return response to request a suspension to another action
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	static FNLEventResponse TrySuspendFor(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::SUSPEND, priority, action, reason, payload);
	}

	// Return response to request this action be done because of this event
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	static FNLEventResponse TryDone(TSubclassOf<class UNLAction> action, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::DONE, priority, action, reason);
	}

protected:
	// The action which started us in the stack that we will resume to when we finish
	UPROPERTY()
	UNLAction* ParentAction;

	// The action which we started
	UPROPERTY()
	UNLAction* ChildAction;

	// The response caused by an event in this action
	// Can be superseeded by other responses of a higher priority
	UPROPERTY()
	FNLEventResponse EventResponse;
};
