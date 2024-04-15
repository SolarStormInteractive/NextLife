// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"

// Baseline event sets
#include "EventSets/NLGeneralEvents.h"
#include "EventSets/NLSensingEvents.h"
#include "EventSets/NLMovementEvents.h"

#include "NLBehavior.generated.h"

// DOCUMENTATION
/**
 * Events
 * -------------------
 * When an event occurs it is iterated to each action starting from the top of the stack. If an action responds to
 * the event, iteration stops and the event response is stored in the action.
 *
 * When a behavior is ticked, events are first processed.
 * The top actions events are immediately processed.
 * Actions under the top are iterated for event responses. The highest priority response is selected and other responses
 * are cleared. If no response was found, nothing happens, and event processing completes.
 * If a response was found, the top action is asked if the response can be used
 */

// On behavior actions complete
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNLOnBehaviorEnded, class UNLBehavior*, endedBehavior);

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Behavior
 * Maintains an action stack, propgates events to actions.
*/
UCLASS(Blueprintable, Abstract)
class NEXTLIFE_API UNLBehavior : public UObject
							   , public INLGeneralEvents
							   , public INLSensingEvents
							   , public INLMovementEvents
{
    GENERATED_BODY()
public:
    UNLBehavior();

	virtual void BeginDestroy() override;

	UPROPERTY(BlueprintAssignable)
	FNLOnBehaviorEnded OnBehaviorEnded;

	// The initial action to create and start on BeginBehavior.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TSubclassOf<class UNLAction> InitialActionClass;

	// Get the owning brain component
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	class UNextLifeBrainComponent* GetBrainComponent() const;

	// Gets the world time associated with the AI being driven by this behavior
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	float GetWorldTimeSeconds() const;

	// Call when this behavior has been restored
	void OnSaveRestored();

	/**
	 * Puts together the current action stack into an array
	 * Return true if the stack is valid (Behavior has begun and had an initial action)
	 * The returned array is ordered as the first action first, the current active action last
	 */
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	bool GetActionStack(TArray<class UNLAction*>& actionStackOut) const;

	// Begins this behavior (creates the initial action and starts it, possibly causing a chain reaction of actions to stack).
	virtual void BeginBehavior();

	// Run this behavior. Called from the NextLife Brain Component.
	virtual void RunBehavior(float deltaSeconds);

	// Sets all events into a paused state.
	// This prevents new events from propagating to actions.
	void SetEventsPausedState(bool pausedState)
	{
		EventsPaused = pausedState;
	}

	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	FString GetBehaviorShortName() const
	{
		if(!BehaviorShortName.IsEmpty())
		{
			return BehaviorShortName;
		}
		return GetClass()->GetName();
	}

	// Returns true if event propagation is currently paused, as in, when an event occurs it is ignored, not pushed through
	// the actions.
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	FORCEINLINE bool AreEventsPaused() const
	{
		return EventsPaused;
	}

	// Returns true if the behavior has begun
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	FORCEINLINE bool HasBehaviorBegun() const
	{
		return Action != nullptr;
	}

	// Returns the top level action on the action stack
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	FORCEINLINE UNLAction* GetAction() const
	{
		return Action;
	}

	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	UNLAction* GetActionOfClass(TSubclassOf<UNLAction> actionClass) const;

	/**
	 * Stops the behavior. Tears down the action stack gracefully by ending each action. Acts like the behavior ended if callBehaviorEnded is true.
	 * @param callBehaviorEnded - Should this call fire the OnBehaviorEnded event?
	 */
	void StopBehavior(bool callBehaviorEnded);

	// Helper function to create a basic general message with a name assigned.
	// It would be cleaner to instead create message classes derived from UNLGeneralMessage
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	UNLGeneralMessage* CreateGeneralNamedMessage(const FName messageName);

	// Helper function to create a basic generic message based on a class.
	// This just creates the message with the outer set to this behavior. You don't need to use this
	// function to create messages.
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	UNLGeneralMessage* CreateGeneralMessage(TSubclassOf<UNLGeneralMessage> messageClass);

	/**
	* INLGeneralEvents Implementation
	*/
	virtual FNLEventResponse General_Message_Implementation(UNLGeneralMessage* message) override;
	
	/**
	* INLSensingEvents Implementation
	*/
	virtual FNLEventResponse Sense_Sight_Implementation(APawn* subject, bool indirect) override;
	virtual FNLEventResponse Sense_SightLost_Implementation(APawn* subject) override;
	virtual FNLEventResponse Sense_Sound_Implementation(APawn *OtherActor, const FVector &Location, float Volume, int32 flags) override;
	virtual FNLEventResponse Sense_Contact_Implementation(AActor* other, const FHitResult& hitResult) override;

	/**
	* INLMovementEvents Implementation
	*/
	virtual FNLEventResponse Movement_MoveTo_Implementation(const AActor *goal, const FVector &pos, float range) override;
	virtual FNLEventResponse Movement_MoveToComplete_Implementation(FAIRequestID RequestID, const EPathFollowingResult::Type Result) override;

protected:

	// A short name for the behavior. Used in the debug spew so it is best to keep this simple and short.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FString BehaviorShortName;

	/**
	 * Applies the current action result to the current TOP action possibly modifying the current set TOP action
	 */
	UNLAction* ApplyActionResult(const struct FNLActionResult& result, bool fromRequest);

	/**
	 * When an event occurs and an action accept it with a result this is called to store the event result for processing
	 * in the future.
	 * @return True if the event was handled
	 */
	UFUNCTION(BlueprintCallable, Category = "NextLife|Behavior")
	bool HandleEventResponse(UNLAction* respondingAction, const FName eventName, const struct FNLEventResponse& response);
	
	/**
	 * Apply pending events in the action stack and return the new top level action
	 */
	class UNLAction* ApplyPendingEvents();

	/**
	 * Creates an action result from an event response
	 * Used when applying events
	 */
	static void CreateActionResultFromEvent(const FNLEventResponse& response, FNLActionResult& actionResultOut);
	
	// The current TOP action
	UPROPERTY(SaveGame) // BlueprintReadOnly, Category = "Behavior", 
	class UNLAction* Action;

	// If paused, events will not be accepted
	UPROPERTY(SaveGame)
	bool EventsPaused;
};
