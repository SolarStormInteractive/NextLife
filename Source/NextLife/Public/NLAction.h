// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"

#include "NLAction.generated.h"

//----------------------------------------------------------------------------------------------------------------------
/**
* An action result structure returned from action Start and Update
*/
USTRUCT(BlueprintType)
struct FNLActionResult
{
	GENERATED_BODY()

	FNLActionResult()
		: Change(ENLActionChangeType::NONE)
		, Payload(nullptr)
	{}

	FNLActionResult(ENLActionChangeType change,
					TSubclassOf<class UNLAction> action,
					const FString& reason,
					class UNLActionPayload* payload = nullptr)
		: Change(change)
		, Action(action)
		, Payload(payload)
		, Reason(reason)
	{}

	// The change to be made
	UPROPERTY()
	ENLActionChangeType Change;

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

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Action : An action is something the AI should do to complete a task
 *               A Behavior starts an initial action and other actions can start from actions creating an action list.
*/
UCLASS(Blueprintable, Abstract)
class NEXTLIFE_API UNLAction : public UObject
{
    GENERATED_BODY()
public:
    UNLAction();

	// Behaviors control us
	friend class UNLBehavior;

	// Get the current short description. Could evolve depending on internal action state.
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	virtual FString GetShortDescription() const
	{
		return ActionShortDescription;
	}

	// Gets the previous action
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	UNLAction* GetPreviousAction() const
	{
		return PreviousAction;
	}

	// Gets the next action
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	UNLAction* GetNextAction() const
	{
		return NextAction;
	}

	// Determine if an action is below me
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	bool IsBelowMe(const UNLAction* otherAction) const
	{
		if(otherAction == this)
		{
			return false;
		}

		UNLAction* previous = PreviousAction;
		while(previous)
		{
			if(previous == otherAction)
			{
				return true;
			}
			previous = previous->PreviousAction;
		}

		return false;
	}

	// Determine if an action is above me
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	bool IsAboveMe(const UNLAction* otherAction) const
	{
		if(otherAction == this)
		{
			return false;
		}

		UNLAction* next = NextAction;
		while(next)
		{
			if(next == otherAction)
			{
				return true;
			}
			next = next->NextAction;
		}

		return false;
	}

protected:

	// A short description about the action. Used in debug spew so it is best to keep this simple, maybe three words max.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString ActionShortDescription;

	/**
	 * Called when this action is about to be serialized (for a save game)
	 * Useful for setting up save game variables (extra information for when the game is loaded to get things back in order)
	 */
	virtual void OnPreSave() {}

	/**
	 * Called when this action has been loaded from serialized data (for loading a saved game)
	 * Useful for when things need to be started again after a saved game is loaded (get back on movement course, etc)
	 */
	virtual void OnLoaded() {}

	/**
	* Starts this action and sets its previous action pointer.
	* This could start a new action to immediately be started which will be reflected in the result.
	*/
	FNLActionResult InvokeOnStart(const UNLActionPayload* payload);

	/**
	 * The primary action update call steps (each step could occur over multiple frames depending changes in state):
	 * 1 If head action has not started, runs the OnStart() call on it
	 * -- If the start call causes a change of action, the new action will be started, and so on and so forth.
	 * --- Frame ends if action starts occured
	 * 2 Processes pending events in the action stack. The top action gets events immediately processes. Burried actions
	 *   are then reviewed to find the most prominent event which occured over the frame. Other non-prominent events are cleared.
	 *   If a prominent event is found, it is requested of the action stack to review the event and accept it. All current actions have
	 *   a chance to reject the events request via the OnRequestEvent() call. If the event requests an action be added to the stack
	 *   which is already on the stack, 
	 * -- If new action begins, it is started. If change_to occurs, old action has End() called.
	 * --- New actions have OnStart() called. If the start call causes a change of action, the new action will be started, and so on and so forth.
	 * ---- If new actions occured, frame ends
	 * 3 Update is called on the current action
	 * -- If update causes a change or suspend, the new action is started, old action is ended if "change", Start is called on the new action.
	 *	  If the start call causes a change of action, the new action will be started, and so on and so forth.
	 *
	 * After all the steps occur, a new head action might be set with previous actions under it.
	 * When an action is suspended for another, the OnSuspend call will be made on that action which is being suspended.
	 * If an action is ended via a OnDone() result or ChangeTo event, the OnDone call will be made.
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
	void InvokeOnDone(const UNLAction* nextAction);

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

	/**
	 * Gets the currently assigned blackboard component (if one has been assigned in AIController via UseBlackboard)
	 * Blackboards can be useful for storing information in a generic fassion for the AI to use while executing.
	 * Passing information to an AI through a blackboard can generalize your AI routines to be usable by many different pawn types.
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	class UBlackboardComponent* GetBlackboard() const;

	/**
	 * Is this action currently the top action
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Action")
	FORCEINLINE bool IsTopAction() const
	{
		return NextAction == nullptr;
	}

	/**
	 * Start the action, the result will be immediately processed which could cause an immediate transition to another action.
	 * If a transition occurs, those new actions will follow the same rule of Start and immediate processing.
	 * Any started actions will end in an OnDone call. If suspended, also expect a OnSuspend call.
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
	* Return true if this is ok, which means the OnUpdate call will be made next frame. Return false if this action
	* should instead be ended, in which case OnDone will be called and the previous action will resume (if there is one).
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	bool OnSuspend(const UNLAction *interruptingAction);
	virtual bool OnSuspend_Implementation(const UNLAction *interruptingAction)
	{
		return true;
	}

	/**
	 * When an Action is resumed after being suspended
	 * The action result will be immediately processed which can request the action be completed, sustained, or changed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	FNLActionResult OnResume(const UNLAction *resumedFromAction);
	virtual FNLActionResult OnResume_Implementation(const UNLAction *resumedFromAction)
	{
		return Continue();
	}

	/**
	 * When a lower level (not top action) requests a change to the stack, the top level action gets this call
	 * and can refuse the event with the return response, true being accept, false being refuse.
	 * NOTE: If this is a simple additive action, it might be meaningful to return true always to let other actions know
	 *		 it can be overriden.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	bool OnRequestEvent(const FNLEventResponse &eventRequested, UNLAction* requester);
	virtual bool OnRequestEvent_Implementation(const FNLEventResponse &eventRequested, UNLAction* requester)
	{
		// By default only allow non-destructive events which are greater priority than TRY:
		// ignore trys (Top level actions should ignore trys for the most part, it means the lower level action doesn't care).
		// ignore anything but suspends (Changes and Dones could cause large changes to the stack, so don't allow them by default).
		// and the suspend should be an appendage (so the request won't remove large parts of the stack).
		return eventRequested.Priority > ENLEventRequestPriority::TRY && eventRequested.IsNonDestructive();
	}

	/**
	 * Asks an action to take over an events request.
	 * If true is returned, this action took the payload and the request should be dropped.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Action")
	bool OnRequestTakeover(const FNLEventResponse &eventRequested, UNLAction* requester);
	virtual bool OnRequestTakeover_Implementation(const FNLEventResponse &eventRequested, UNLAction* requester)
	{
		// By default no takeovers are accepted.
		return false;
	}

	/**
	 * Continue, no change
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	FNLActionResult Continue()
	{
		return FNLActionResult();
	}

	/**
	 * Change this action to a new action
	 * OnDone will be called, and OnStart will be called on the new action replacing this action.
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	FNLActionResult ChangeTo(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionChangeType::CHANGE, action, reason, payload);
	}

	/**
	 * Suspend this action for another. Puts a new action at the top of the stack, and suspends us, calling OnSuspend.
	 * OnResume will be called when this action resumes (if it isn't being terminated from a Done event)
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	FNLActionResult SuspendFor(TSubclassOf<class UNLAction> action, UNLActionPayload* payload, const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionChangeType::SUSPEND, action, reason, payload);
	}

	// The action is done
	UFUNCTION(BlueprintPure, Category = "NextLife|Action Result")
	FNLActionResult Done(const FString& reason = TEXT(""))
	{
		return FNLActionResult(ENLActionChangeType::DONE, nullptr, reason);
	}

	// Return response to continue (no request being made, let the parent actions handle this event)
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TryContinue()
	{
		return FNLEventResponse();
	}

	/**
	 * Return response to request a change to another action
	 * This request is destructive as it would change the current action (which could be burried) to a new action (terminating all actions above)
	 * All actions above the action requesting this change
	 * @param action - The action class to changeto for
	 * @param payload - The payload for the action to changeto for
	 * @param priority - The priority of this request. Higher priorities will override lower priorities. Use this as an importance factor. Unimportant things should just be TRY.
	 * @param reason - The reason for this changeto, for debug purposes
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TryChangeTo(TSubclassOf<class UNLAction> action, UNLActionPayload* payload,
								 const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY,
								 const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLActionChangeType::CHANGE, priority, action, reason, payload);
	}

	/**
	 * Return response to request a suspension to another action
	 * Suspends will occur even if this action is not active. The new action will be pushed to the top of the stack.
	 * @param action - The action class to suspend for
	 * @param payload - The payload for the action to suspend for
	 * @param priority - The priority of this request. Higher priorities will override lower priorities. Use this as an importance factor. Unimportant things should just be TRY.
	 * @param reason - The reason for this suspend, for debug purposes
	 * @param suspendBehavior - The type of behavior when trying to suspend. See ENLSuspendBehavior for details about the different behaviors.
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TrySuspendFor(TSubclassOf<class UNLAction> action, UNLActionPayload* payload,
								   const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY,
								   const FString& reason = TEXT(""), const ENLSuspendBehavior suspendBehavior = ENLSuspendBehavior::NORMAL)
	{
		return FNLEventResponse(ENLActionChangeType::SUSPEND, priority, action, reason, payload, suspendBehavior);
	}

	/**
	 * Return response to request this action be done because of this event
	 * If this action is burried under other actions, Done will happen once this action becomes the active action again.
	 * @param priority - The priority of this request. Higher priorities will override lower priorities. Use this as an importance factor. Unimportant things should just be TRY.
	 * @param reason - The reason for this done, for debug purposes
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Event Response")
	FNLEventResponse TryDone(const ENLEventRequestPriority priority = ENLEventRequestPriority::TRY, const FString& reason = TEXT(""))
	{
		return FNLEventResponse(ENLActionChangeType::DONE, priority, nullptr, reason);
	}

private:

	// Has this action had its OnStart function called yet?
	UPROPERTY()
	bool HasStarted;
	
	// The action which started us in the stack that we will resume to when we finish
	UPROPERTY()
	UNLAction* PreviousAction;

	// The action which we started
	UPROPERTY()
	UNLAction* NextAction;

	// The response caused by an event in this action
	// Can be superseeded by other action event responses of a higher priority
	UPROPERTY()
	FNLEventResponse EventResponse;
};
