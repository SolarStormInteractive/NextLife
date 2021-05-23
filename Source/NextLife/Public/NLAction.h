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

	// Behaviors control us
	friend class UNLBehavior;

protected:

	// The type of payload expected by this action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NextLife|Action")
	TSubclassOf<UNLActionPayload> PayloadClass;

	// This action will only be suspendable from an event IF the suspend request is higher than this priority.
	// If a suspend event is dropped because of this, a verbose log message will be thrown.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NextLife|Action")
	ENLEventRequestPriority SuspendPriority;

	/**
	* Starts this action and sets its previous action pointer.
	* This could start a new action to immediately be started which will be reflected in the result.
	*/
	FNLActionResult InvokeOnStart(const UNLActionPayload* payload);

	/**
	 * The primary action update call steps (each step could occur over multiple frames depending changes in state):
	 * 1 If head action has not started, runs the Start() call on it
	 * -- If the start call causes a change of action, the new action will be started, and so on and so forth.
	 * --- Frame ends if action starts occured
	 * 2 Processes pending events in the action stack, runs suspends if possible (which start new actions)
	 * -- If new action begins, it is started. If change_to occurs, old action has End() called.
	 * --- New actions have Start() called. If the start call causes a change of action, the new action will be started, and so on and so forth.
	 * ---- If new actions occured, frame ends
	 * 3 Update is called on the current action
	 * -- If update causes a change or suspend, the new action is started, old action is ended if "change", Start is called on the new action.
	 *	  If the start call causes a change of action, the new action will be started, and so on and so forth.
	 *
	 * After all the steps occur, a new head action might be set with previous actions under it.
	 * When an action is suspended for another, the OnSuspend call will be made on that action which is being suspended.
	 * If an action is ended via a Done() result or ChangeTo event, the OnDone call will be made.
	 */
	FNLActionResult InvokeUpdate(float deltaSeconds);

	/**
	* Suspends this action possibly causing the action to complete. If this invoke returns false, the suspend cannot
	* occur and the action should be ended. Any action which return false on suspend should expect an OnEnd invoke shortly after.
	*/
	bool InvokeOnSuspend(const UNLAction *interruptingAction);

	/**
	 * Called when this action resumes after being suspended. The result can contain any action you would like to take
	 * such as immediately ending, changing to another action or suspending again.
	 */
	FNLActionResult InvokeOnResume(const UNLAction *resumingFrom);

	/**
	* Ends the action and action children and any actions above this action.
	*/
	void InvokeOnDone(const UNLAction* nextAction, const bool endAboveActions = true);

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

	// Gets the world time associated with the AI being driven by this actions behavior
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	float GetWorldTimeSeconds() const;

	// Gets the currently assigned blackboard component (if one has been assigned in AIController via UseBlackboard)
	// Blackboards can be useful for storing information in a generic fassion for the AI to use while executing.
	// Passing information to an AI through a blackboard can generalize your AI routines to be usable by many different pawn types.
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class UBlackboardComponent* GetBlackboard() const;

	// Adds a sub action and calls OnStart
	//UFUNCTION(BlueprintCallable, Category = "NextLife|Action")
	//void AddSubAction(TSubclassOf<UNLAction> subActionClass, UNLActionPayload* payload) {}

	// Removes a sub action and calls OnEnd
	//UFUNCTION(BlueprintCallable, Category = "NextLife|Action")
	//void RemoveSubAction(TSubclassOf<UNLAction> subActionClass) {}

	//-----------------------------------------------------------------------------------------
	/**
	 * Start the action, the result will be immediately processed which could cause an immediate transition to another action.
	 * If a transition occurs, those new actions will follow the same rule of Start and immediate processing.
	 * Any started actions will end in an OnEnd call. If suspended, also expect a OnSuspend call.
	 * When an action is resumed from another action, expect a OnResume call.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	FNLActionResult OnStart(const UNLActionPayload* payload);
	virtual FNLActionResult OnStart_Implementation(const UNLActionPayload* payload)
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
	void OnDone(const UNLAction *nextAction);
	virtual void OnDone_Implementation(const UNLAction *nextAction)
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
	FNLEventResponse TryContinue()
	{
		return FNLEventResponse();
	}

	// Request that event propagation past this point is prevented. Allows actions to block previous actions from taking a crack at an event.
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TrySustain(const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::SUSTAIN, priority, nullptr, reason);
	}

	// Return response to request a change to another action
	// If this action is burried under other actions, ChangeTo will happen once this action becomes the active action again.
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TryChangeTo(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::CHANGE, priority, action, reason, payload);
	}

	// Return response to request a suspension to another action
	// Suspends will occur even if this action is not active. The new action will be pushed to the top of the stack.
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TrySuspendFor(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::SUSPEND, priority, action, reason, payload);
	}

	// Return response to request this action be done because of this event
	// If this action is burried under other actions, Done will happen once this action becomes the active action again.
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TryDone(TSubclassOf<class UNLAction> action, const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLEventRequest::DONE, priority, action, reason);
	}

	// Has this action had its OnStart function called yet?
	UPROPERTY()
	bool HasStarted;
	
	// The action which started us in the stack that we will resume to when we finish
	UPROPERTY()
	UNLAction* PreviousAction;

	// The action which we started
	UPROPERTY()
	UNLAction* NextAction;

	// Actions which run along side us. Can be useful for creating a base action with smaller sub actions.
	// Sub actions do not accept events.
	//UPROPERTY()
	//TArray<UNLAction*> SubActions;

	// The response caused by an event in this action
	// Can be superseeded by other action event responses of a higher priority
	UPROPERTY()
	FNLEventResponse EventResponse;
};
