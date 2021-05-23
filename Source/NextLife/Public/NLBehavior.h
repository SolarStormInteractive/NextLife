// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

// Baseline event sets
#include "EventSets/NLGeneralEvents.h"
#include "EventSets/NLInflictionEvents.h"
#include "EventSets/NLSensingEvents.h"
#include "EventSets/NLMovementEvents.h"

#include "NLBehavior.generated.h"

// On behavior actions complete
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNLOnBehaviorEnded, class UNLBehavior*, endedBehavior);

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Behavior : The base behavior of a NextLife AI. This should be extended to fit the behavior of the AI you are trying to create.
 * See example behaviors in the Behaviors folder.
 * How it all works:
 * A behavior is how an AI should act under certain circumstances. It may be as simple as how the AI acts while idle.
*/
UCLASS(Blueprintable)
class NEXTLIFE_API UNLBehavior : public UObject
							   , public INLGeneralEvents
							   , public INLInflictionEvents
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

	// Begins this behavior (creates the initial action and starts it, possibly causing a chain reaction of actions to stack).
	void BeginBehavior();

	// Run this behavior. Called from the NextLife Brain Component.
	void RunBehavior(float deltaSeconds);

	// Sets all events into a paused state.
	// This prevents new events from propagating to actions.
	void SetEventsPausedState(bool pausedState)
	{
		EventsPaused = pausedState;
	}

	// Returns true if event propagation is currently paused, as in, when an event occurs it is ignored, not pushed through
	// the actions.
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	FORCEINLINE bool AreEventsPaused() const
	{
		return EventsPaused;
	}

	// Ends the behavior. Tears down the action stack gracefully by ending each action.
	void EndBehavior();

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
	virtual FNLEventResponse Movement_MoveToComplete_Implementation(FAIRequestID RequestID) override;

protected:

	/**
	 * Applies the current action result to the current TOP action possibly modifying the current set TOP action
	 */
	UNLAction* ApplyActionResult(const struct FNLActionResult& result);

	/**
	 * When an event occurs and an action accept it with a result this is called to store the event result for processing
	 * in the future.
	 */
	UFUNCTION(BlueprintCallable, Category = "NextLife|Behavior")
	void StorePendingEventResult(UNLAction* respondingAction, const FString& eventName, const struct FNLEventResponse& result);

	struct FNLPendingEvent
	{
		FNLPendingEvent(const FNLEventResponse& response, class UNLAction* respondingAction)
			: Response(response), RespondingAction(respondingAction) {}
		FNLEventResponse Response;
		class UNLAction* RespondingAction;
	};
	
	/**
	 * Processes events in the action stack
	 */
	FNLPendingEvent ProcessPendingEvents() const;

	/**
	* Applies an events result which could cause a change to the action stack
	*/
	UNLAction* ApplyEventResponse(const FNLPendingEvent& result);

	// The current TOP action
	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	class UNLAction* Action;

	// If paused, events will not be accepted
	UPROPERTY()
	bool EventsPaused;
};
