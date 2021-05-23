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

	// Process events before updating
	const UNLBehavior::FNLPendingEvent eventResponse = ProcessPendingEvents();
	if(eventResponse.Response.Request != ENLEventRequest::NONE && eventResponse.RespondingAction)
	{
		// A change is occuring, apply it
		Action = ApplyEventResponse(eventResponse);
		if(!Action)
		{
			OnBehaviorEnded.Broadcast(this);
		}
		return; // Frame skip
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
	
	if(GetBrainComponent()->LogState && result.Result != ENLActionResultType::CONTINUE)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		UE_LOG(LogNextLife, Warning, TEXT("ApplyActionResult : %3.2f: %s:%s: "), GetWorldTimeSeconds(),
															*GetBrainComponent()->GetAIOwner()->GetName(), 
															*GetName());
		CLEAR_WARN_COLOR();
	}
	
	switch(result.Result)
	{
		case ENLActionResultType::CHANGE:
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
		case ENLActionResultType::SUSPEND:
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
		case ENLActionResultType::DONE:
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
void UNLBehavior::StorePendingEventResult(UNLAction* respondingAction, const FString& eventName, const FNLEventResponse& result)
{
	check(respondingAction);
	check(result.Request != ENLEventRequest::NONE);

	FString storeAction = "STORED";

	if(result.Request == ENLEventRequest::SUSPEND && result.Priority < Action->SuspendPriority)
	{
		UE_LOG(LogNextLife, Verbose, TEXT("%s::%s -> %s SUSPEND ignored because of TOP action SuspendPriority"), *GetName(), *respondingAction->GetName(), *eventName);
		storeAction = TEXT("SUSPEND IGNORED");
	}
	else if(result.Priority > respondingAction->EventResponse.Priority)
	{
		if(respondingAction->EventResponse.Priority != ENLEventRequestPriority::NONE)
		{
			storeAction = "OVERRODE PREVIOUS WITH";
		}
		respondingAction->EventResponse = result;
	}
	else if(result.Priority == respondingAction->EventResponse.Priority && respondingAction->EventResponse.Request == ENLEventRequest::SUSTAIN)
	{
		// Sustains can be overriden
		storeAction = TEXT("OVERRODE SUSTAIN WITH");
		respondingAction->EventResponse = result;
	}
	else
	{
		storeAction = TEXT("IGNORED");
		if(result.Priority == ENLEventRequestPriority::CRITICAL)
		{
			UE_LOG(LogNextLife, Warning, TEXT("%s::%s -> %s RESULT_CRITICAL collision"), *GetName(), *respondingAction->GetName(), *eventName);
			storeAction = TEXT("IGNORE COLLISION");
		}
	}
	
	if(GetBrainComponent()->LogState)
	{
		FString requestStr;
		switch(result.Request)
		{
			case ENLEventRequest::DONE:
				{
					requestStr = TEXT("DONE");
					break;
				}
			case ENLEventRequest::CHANGE:
				{
					check(result.Action);
					requestStr = FString::Printf(TEXT("CHANGE to %s (%s)"), *result.Action->GetName(), *UEnum::GetValueAsString(result.Priority));
					break;
				}
			case ENLEventRequest::SUSPEND:
				{
					check(result.Action);
					requestStr = FString::Printf(TEXT("SUSPEND for %s (%s)"), *result.Action->GetName(), *UEnum::GetValueAsString(result.Priority));
					break;
				}
			case ENLEventRequest::TAKE_OVER:
				{
					check(result.Action);
					requestStr = FString::Printf(TEXT("%s TAKE_OVER (%s)"), *result.Action->GetName(), *UEnum::GetValueAsString(result.Priority));
					break;
				}
			default:
				return;
		}
		SET_WARN_COLOR(COLOR_CYAN);
		UE_LOG(LogNextLife, Warning, TEXT("%s:%s %s EVENT '%s' with request %s. %s"), *GetName(),
																					  *respondingAction->GetName(),
																					  *storeAction,
																					  *eventName,
																					  *requestStr,
																					  *result.Reason);
		CLEAR_WARN_COLOR();
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLBehavior::FNLPendingEvent UNLBehavior::ProcessPendingEvents() const
{
	if(Action->EventResponse.IsRequestingChange())
	{
		// Apply this top level response immediately, and clear
		const FNLEventResponse eventResponse = Action->EventResponse;
		Action->EventResponse = FNLEventResponse();
		
		if(eventResponse.Request == ENLEventRequest::TAKE_OVER)
		{
			// TOP action asked for a take over but there is nothing to take over. Just stop here for this frame.
			return FNLPendingEvent(FNLEventResponse(), nullptr);
		}
		return FNLPendingEvent(eventResponse, Action);
	}

	// Clear out the top, any other response doesn't change anything
	Action->EventResponse = FNLEventResponse();

	// Check for pending suspends in previous actions
	UNLAction* PreviousAction = Action->PreviousAction;
	while(PreviousAction)
	{
		// Suspends add ontop of the current TOP action
		if(PreviousAction->EventResponse.Request == ENLEventRequest::SUSPEND)
		{
			const FNLEventResponse eventResponse = PreviousAction->EventResponse;
			PreviousAction->EventResponse = FNLEventResponse();
			return FNLPendingEvent(eventResponse, Action);
		}
		// Take overs clobber actions above a specific action making that action the TOP action
		if(PreviousAction->EventResponse.Request == ENLEventRequest::TAKE_OVER)
		{
			const FNLEventResponse eventResponse = PreviousAction->EventResponse;
			PreviousAction->EventResponse = FNLEventResponse();
			return FNLPendingEvent(eventResponse, PreviousAction);
		}

		PreviousAction = PreviousAction->PreviousAction;
	}

	// No new events
	return FNLPendingEvent(FNLEventResponse(), nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction* UNLBehavior::ApplyEventResponse(const FNLPendingEvent& result)
{
	checkf(Action, TEXT("ApplyEventResponse should not be made without a valid action stack!"));
	checkf(result.RespondingAction, TEXT("ApplyEventResponse result should have a valid RespondingAction!"));
	
	if(GetBrainComponent()->LogState && result.Response.Request != ENLEventRequest::NONE)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		UE_LOG(LogNextLife, Warning, TEXT("ApplyEventResponse : %3.2f: %s:%s: "), GetWorldTimeSeconds(),
															*GetBrainComponent()->GetAIOwner()->GetName(), 
															*GetName());
		CLEAR_WARN_COLOR();
	}
	
	switch(result.Response.Request)
	{
		case ENLEventRequest::CHANGE:
			{
				if(!result.Response.Action)
				{
					UE_LOG(LogNextLife, Error, TEXT("CHANGE to a nullptr Action"));
					return Action;
				}
				
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_GREEN);
					UE_LOG(LogNextLife, Warning, TEXT("%s CHANGE to %s"), *result.RespondingAction->GetName(), 
																		  *result.Response.Action->GetName());
					CLEAR_WARN_COLOR();
				}

				
				
				return Action;
			}
		case ENLEventRequest::SUSPEND:
			{
				if(!result.Response.Action)
				{
					UE_LOG(LogNextLife, Error, TEXT("SUSPEND to a nullptr Action"));
					return Action;
				}
				
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_YELLOW);
					UE_LOG(LogNextLife, Warning, TEXT("%s SUSPEND to %s"), *result.RespondingAction->GetName(), 
																		   *result.Response.Action->GetName());
					CLEAR_WARN_COLOR();
				}
				
				return Action;
			}
		case ENLEventRequest::DONE:
			{
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_RED);
					UE_LOG(LogNextLife, Warning, TEXT("%s DONE"), *result.RespondingAction->GetName());
					CLEAR_WARN_COLOR();
				}

				if(result.RespondingAction == Action)
				{
					// The top level action is ending, do the normal stuff
					UNLAction* previousTopAction = Action;
					Action = Action->PreviousAction;

					previousTopAction->InvokeOnDone(Action);

					const FNLActionResult resumeResult = Action->InvokeOnResume(previousTopAction);
					return ApplyActionResult(resumeResult);
				}
				else
				{
					// A middle action is ending, remove it and clean up links
					
				}
				
				return Action;
			}
		case ENLEventRequest::TAKE_OVER:
			{
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_RED);
					UE_LOG(LogNextLife, Warning, TEXT("%s TAKE_OVER"), *result.RespondingAction->GetName());
					CLEAR_WARN_COLOR();
				}

				UNLAction* previousTopAction = Action;
				
				// The responding action becomes the new action and everything above is DONE
				Action = result.RespondingAction;
				
				if(Action->NextAction)
				{
					// Make all above actions DONE
					Action->NextAction->InvokeOnDone(Action);
					Action->NextAction = nullptr;
					
					// Call the resume as usual which could cause new actions etc
					const FNLActionResult resumeResult = Action->InvokeOnResume(previousTopAction);
					return ApplyActionResult(resumeResult);
				}
				
				// Nothing to resume from so just continue
				return Action;
			}
		case ENLEventRequest::NONE:
		case ENLEventRequest::SUSTAIN:
		default:
			// Nones and Sustains do nothing
			return Action;
	}
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
    		if(responseOut.Request != ENLEventRequest::NONE)
    		{
    			StorePendingEventResult(curAction, TEXT("General_Message"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Sense_Sight"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Sense_SightLost"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Sense_Sound"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Sense_Contact"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Movement_MoveTo"), responseOut);
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
			if(responseOut.Request != ENLEventRequest::NONE)
			{
				StorePendingEventResult(curAction, TEXT("Movement_MoveToComplete"), responseOut);
				break;
			}
		}

		curAction = curAction->PreviousAction;
	}

	return responseOut;
}
