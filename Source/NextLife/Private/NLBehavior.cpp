// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NLBehavior.h"
#include "NextLifeBrainComponent.h"
#include "AIController.h"
#include "NextLifeModule.h"
#include "NLAction.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLBehavior::UNLBehavior()
	: EventsPaused(false)
{

}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::BeginDestroy()
{
	Super::BeginDestroy();
	EndBehavior();
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNextLifeBrainComponent* UNLBehavior::GetBrainComponent() const
{
	return Cast<UNextLifeBrainComponent>(GetOuter());
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
float UNLBehavior::GetWorldTimeSeconds() const
{
	UNextLifeBrainComponent* brainComponent = GetBrainComponent();
	if(brainComponent)
	{
		if(brainComponent->GetAIOwner())
		{
			return brainComponent->GetAIOwner()->GetWorld()->GetTimeSeconds();
		}
	}

	return -1.0f;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::BeginBehavior()
{
	if(!InitialActionClass)
	{
		UE_LOG(LogNextLife, Error, TEXT("Trying to start a behavior which has no initial action class? Set InitialActionClass in behavior '%s'"), *GetName());
		return;
	}

	// Create the initial action
	Action = NewObject<UNLAction>(this, InitialActionClass);
	check(Action);

	// The action hasn't started yet, start it and apply the result
	const FNLActionResult actionResult = Action->InvokeOnStart(nullptr);
	Action = ApplyActionResult(actionResult);

	if(!Action)
	{
		OnBehaviorEnded.Broadcast(this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::RunBehavior(float deltaSeconds)
{
	if(!Action || !Action->HasStarted)
	{
		// This is an error case, but the error message would have been thrown by now.
		return;
	}

	// Apply pending events which could modify the current action
	Action = ApplyPendingEvents();
	if(!Action)
	{
		OnBehaviorEnded.Broadcast(this);
		return;
	}

	// Frame Update the current action and apply its result
	const FNLActionResult actionResult = Action->InvokeUpdate(deltaSeconds);
	Action = ApplyActionResult(actionResult);

	if(!Action)
	{
		OnBehaviorEnded.Broadcast(this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::EndBehavior()
{
	if(!Action || !Action->HasStarted)
	{
		// Already in an ended state
		Action = nullptr;
		return;
	}

	// Get the root action
	UNLAction* rootAction = Action;
	while(rootAction->PreviousAction)
	{
		rootAction = rootAction->PreviousAction;
	}

	// End it
	rootAction->InvokeOnDone(nullptr);

	// GC will get all the actions
	Action = nullptr;

	OnBehaviorEnded.Broadcast(this);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction* UNLBehavior::ApplyActionResult(const FNLActionResult& result)
{
	checkf(Action, TEXT("ApplyActionResult should not be made without a valid action stack!"));
	checkf(!Action->NextAction, TEXT("The TOP action should not have a NextAction set, something bad happened"));
	
	if(GetBrainComponent()->LogState && result.Change != ENLActionChangeType::NONE)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		UE_LOG(LogNextLife, Warning, TEXT("ApplyActionResult : %3.2f: %s:%s: "), GetWorldTimeSeconds(),
															*GetBrainComponent()->GetAIOwner()->GetName(), 
															*GetName());
		CLEAR_WARN_COLOR();
	}
	
	switch(result.Change)
	{
		case ENLActionChangeType::CHANGE:
			{
				if(!result.Action)
				{
					UE_LOG(LogNextLife, Error, TEXT("CHANGE to a nullptr Action"));
					return Action;
				}

				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_GREEN);
					UE_LOG(LogNextLife, Warning, TEXT("%s CHANGE to %s"), *Action->GetName(), 
																		  *result.Action->GetName());
					CLEAR_WARN_COLOR();
				}

				// Create the new action
				UNLAction* newAction = NewObject<UNLAction>(this, result.Action);
				check(newAction);

				// End the current action
				Action->InvokeOnDone(newAction);

				// Commit the new action as head
				UNLAction* previousAction = Action->PreviousAction;
				Action = newAction;
				Action->PreviousAction = previousAction;

				// Start the new action and apply the result which could cause several actions to start via the recursion.
				const FNLActionResult newActionResult = Action->InvokeOnStart(result.Payload);
				return ApplyActionResult(newActionResult);
			}
		case ENLActionChangeType::SUSPEND:
			{
				if(result.Action == nullptr)
				{
					UE_LOG(LogNextLife, Error, TEXT("SUSPEND to a nullptr Action"));
					return Action;
				}

				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_YELLOW);
					UE_LOG(LogNextLife, Warning, TEXT("%s SUSPEND for %s"), *Action->GetName(), 
																		    *result.Action->GetName());
					CLEAR_WARN_COLOR();
				}

				// Create the new action
				UNLAction* newAction = NewObject<UNLAction>(this, result.Action);
				check(newAction);

				if(Action->InvokeOnSuspend(newAction))
				{
					// Suspended successfully, set the new action as the TOP above the suspended action
					newAction->PreviousAction = Action;
					Action = newAction;
				}
				else
				{
					// Could not suspend this action, it should end
					Action->InvokeOnDone(newAction);
					
					// Commit the new action as head
					UNLAction* previousAction = Action->PreviousAction;
					Action = newAction;
					Action->PreviousAction = previousAction;
				}

				// Start the new action and apply the result which could cause several actions to start via the recursion.
				const FNLActionResult newActionResult = Action->InvokeOnStart(result.Payload);
				return ApplyActionResult(newActionResult);
			}
		case ENLActionChangeType::DONE:
			{
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_RED);
					UE_LOG(LogNextLife, Warning, TEXT("%s DONE"), *Action->GetName());
					CLEAR_WARN_COLOR();
				}

				UNLAction* resumingAction = Action->PreviousAction;
				Action->InvokeOnDone(resumingAction);

				if(resumingAction)
				{
					// Resume the action and let it apply an action result
					UNLAction* endedAction = Action;
					Action = resumingAction;
					Action->NextAction = nullptr;
					
					const FNLActionResult resumeResult = Action->InvokeOnResume(endedAction);
					return ApplyActionResult(resumeResult);
				}

				// No more actions, this behavior has completed!
				Action = nullptr;
				return nullptr;
			}
		default:
			// No change to the current action
			return Action;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNLBehavior::HandleEventResponse(UNLAction* respondingAction, const FName eventName, const FNLEventResponse& response)
{
	check(respondingAction);

	if(response.IsNone())
	{
		// Nothing to handle, move on
		return false;
	}
	
	bool eventHandled = false;
	FString storeAction = "STORED";

	// Check if there is already an event pending which has a higher priority. If not, we can replace it.
	if(response.Priority > respondingAction->EventResponse.Priority)
	{
		if(respondingAction->EventResponse.Priority != ENLEventRequestPriority::NONE)
		{
			storeAction = "OVERRODE PREVIOUS WITH";
		}
		respondingAction->EventResponse = response;
		respondingAction->EventResponse.EventName = eventName;
		eventHandled = true;
	}
	else
	{
		storeAction = TEXT("IGNORED");
		if(response.Priority == ENLEventRequestPriority::CRITICAL)
		{
			UE_LOG(LogNextLife, Warning, TEXT("%s::%s -> %s RESULT_CRITICAL collision"), *GetName(), *respondingAction->GetName(), *eventName.ToString());
			storeAction = TEXT("IGNORE COLLISION");
		}
	}
	
	if(GetBrainComponent()->LogState)
	{
		FString requestStr;
		switch(response.ChangeRequest)
		{
			case ENLActionChangeType::DONE:
				{
					requestStr = TEXT("DONE");
					break;
				}
			case ENLActionChangeType::CHANGE:
				{
					check(response.Action);
					requestStr = FString::Printf(TEXT("CHANGE to %s (%s)"), *response.Action->GetName(), *UEnum::GetValueAsString(response.Priority));
					break;
				}
			case ENLActionChangeType::SUSPEND:
				{
					check(response.Action);
					requestStr = FString::Printf(TEXT("SUSPEND for %s (%s)"), *response.Action->GetName(), *UEnum::GetValueAsString(response.Priority));
					break;
				}
			/*case ENLActionChangeType::TAKE_OVER:
				{
					check(response.Action);
					requestStr = FString::Printf(TEXT("%s TAKE_OVER (%s)"), *response.Action->GetName(), *UEnum::GetValueAsString(response.Priority));
					break;
				}*/
			default:
				break;
		}
		SET_WARN_COLOR(COLOR_CYAN);
		UE_LOG(LogNextLife, Warning, TEXT("%s:%s %s EVENT '%s' with request %s. %s"), *GetName(),
																					  *respondingAction->GetName(),
																					  *storeAction,
																					  *eventName.ToString(),
																					  *requestStr,
																					  *response.Reason);
		CLEAR_WARN_COLOR();
	}

	return eventHandled;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction* UNLBehavior::ApplyPendingEvents()
{
	if(!Action->EventResponse.IsNone())
	{
		// Apply the top level response immediately
		FNLActionResult newAction;
		CreateActionResultFromEvent(Action->EventResponse, newAction);
		Action = ApplyActionResult(newAction);
		// Clear
		Action->EventResponse = FNLEventResponse();
	}

	// Check for pending requests from lower actions, determine the highest order request and send it to the top level
	// for evaluation.
	FNLEventResponse requestedResponse;
	UNLAction* requestingAction = nullptr;
	UNLAction* previousAction = Action->PreviousAction;
	while(previousAction)
	{
		if(previousAction->EventResponse.Priority > requestedResponse.Priority)
		{
			requestedResponse = previousAction->EventResponse;
			requestingAction = previousAction;
		}

		// Clear
		previousAction->EventResponse = FNLEventResponse();
	}

	// Now if this request is not None request from the top this event action to our requesting action to see if the
	// above actions agree to it being executed.
	if(!requestedResponse.IsNone())
	{
		UNLAction* nextAction = Action;
		while(nextAction && nextAction != requestingAction)
		{
			// If any action doesn't agree, we cannot use this request
			if(!Action->OnRequestEvent(requestedResponse, requestingAction))
			{
				break;
			}
			nextAction = nextAction->PreviousAction;
		}

		// If all actions up to the requesting action agree with the event, we clear all actions after the requesting
		// action and run the event.
		if(nextAction == requestingAction)
		{
			// Clear all actions above the requesting action
			requestingAction->NextAction->InvokeOnDone(requestingAction);
			requestingAction->NextAction = nullptr;
			// The requesting action has become the top action for now
			Action = requestingAction;
			// Now run the event
			FNLActionResult newAction;
			CreateActionResultFromEvent(requestedResponse, newAction);
			Action = ApplyActionResult(newAction);
		}
	}
	
	return Action;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::CreateActionResultFromEvent(const FNLEventResponse& response, FNLActionResult& actionResultOut)
{
	actionResultOut.Action = response.Action;
	actionResultOut.Change = response.ChangeRequest;
	actionResultOut.Payload = response.Payload;
	actionResultOut.Reason = response.Reason;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLGeneralMessage* UNLBehavior::CreateGeneralNamedMessage(const FName messageName)
{
	UNLGeneralMessage* newMessage = NewObject<UNLGeneralMessage>(this, UNLGeneralMessage::StaticClass());
	check(newMessage);
	newMessage->MessageName = messageName;
	return newMessage;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLGeneralMessage* UNLBehavior::CreateGeneralMessage(TSubclassOf<UNLGeneralMessage> messageClass)
{
	UNLGeneralMessage* newMessage = NewObject<UNLGeneralMessage>(this, messageClass);
	check(newMessage);
	return newMessage;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::General_Message_Implementation(UNLGeneralMessage* message)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
    {
    	if(curAction->Implements<UNLGeneralEvents>())
    	{
    		responseOut = INLGeneralEvents::Execute_General_Message(curAction, message);
    		if(HandleEventResponse(curAction, TEXT("General_Message"), responseOut))
    		{
    			break;
    		}
    	}

		curAction = curAction->PreviousAction;
    }

    return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sight_Implementation(APawn* subject, bool indirect)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLSensingEvents::Execute_Sense_Sight(curAction, subject, indirect);
			if(HandleEventResponse(curAction, TEXT("Sense_Sight"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_SightLost_Implementation(APawn* subject)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLSensingEvents::Execute_Sense_SightLost(curAction, subject);
			if(HandleEventResponse(curAction, TEXT("Sense_SightLost"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sound_Implementation(APawn* OtherActor, const FVector& Location, float Volume, int32 flags)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLSensingEvents::Execute_Sense_Sound(curAction, OtherActor, Location, Volume, flags);
			if(HandleEventResponse(curAction, TEXT("Sense_Sound"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Contact_Implementation(AActor* other, const FHitResult& hitResult)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLSensingEvents::Execute_Sense_Contact(curAction, other, hitResult);
			if(HandleEventResponse(curAction, TEXT("Sense_Contact"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Movement_MoveTo_Implementation(const AActor* goal, const FVector& pos, float range)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLMovementEvents::Execute_Movement_MoveTo(curAction, goal, pos, range);
			if(HandleEventResponse(curAction, TEXT("Movement_MoveTo"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Movement_MoveToComplete_Implementation(FAIRequestID RequestID)
{
	FNLEventResponse responseOut;
	UNLAction* curAction = Action;
	while(curAction)
	{
		if(curAction->Implements<UNLSensingEvents>())
		{
			responseOut = INLMovementEvents::Execute_Movement_MoveToComplete(curAction, RequestID);
			if(HandleEventResponse(curAction, TEXT("Movement_MoveToComplete"), responseOut))
			{
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}
