// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

// Baseline event sets
#include "EventSets/NLGeneralEvents.h"
#include "EventSets/NLInflictionEvents.h"
#include "EventSets/NLSensingEvents.h"

#include "NLBehavior.generated.h"

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
{
    GENERATED_BODY()
public:
    UNLBehavior();

	// Get the owning brain component
	UFUNCTION(BlueprintPure, Category = "NextLife|Behavior")
	class UNextLifeBrainComponent* GetBrainComponent() const;

	// Begins this behavior (creates the initial action but does not start it. It will start on first RunBehavior).
	void BeginBehavior() {}

	// Run this behavior. Called from the NextLife Brain Component.
	void RunBehavior(float deltaSeconds);

	// Sets all events into a paused state.
	// This prevents new events from propagating to actions.
	void SetEventsPausedState(bool pausedState)
	{
		EventsPaused = pausedState;
	}

	// Ends the behavior. Tears down the action stack gracefully by ending each action.
	void EndBehavior() {}

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

protected:

	// The current HEAD action
	UPROPERTY()
	class UNLAction* Action;

	// If paused, events will not be accepted
	UPROPERTY()
	bool EventsPaused;
};
