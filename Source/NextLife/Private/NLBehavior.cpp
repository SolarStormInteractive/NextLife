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
	StopBehavior(true);
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
bool UNLBehavior::GetActionStack(TArray<UNLAction*>& actionStackOut) const
{
	actionStackOut.Reset(20);
	if(!Action)
	{
		return false;
	}

	UNLAction* nextAction = Action;
	while(nextAction)
	{
		actionStackOut.Add(nextAction);
		nextAction = nextAction->PreviousAction;
	}

	return true;
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
	Action = ApplyActionResult(actionResult, false);

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
	Action = ApplyActionResult(actionResult, false);

	if(!Action)
	{
		OnBehaviorEnded.Broadcast(this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::StopBehavior(bool callBehaviorEnded)
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
	// NOTE: OnDone is not called if the owning pawn is gone (important rule, action functons can always rely on the owner pawn being valid)
	//		 In the case of the pawn being destroyed, OnDone is not called, the action stack is just destroyed.
	if(rootAction->GetPawnOwner() != nullptr)
	{
		rootAction->InvokeOnDone(nullptr);
	}

	// GC will get all the actions
	Action = nullptr;

	if(callBehaviorEnded)
	{
		OnBehaviorEnded.Broadcast(this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction* UNLBehavior::ApplyActionResult(const FNLActionResult& result, bool fromRequest)
{
	checkf(Action, TEXT("ApplyActionResult should not be made without a valid action stack!"));
	checkf(!Action->NextAction, TEXT("The TOP action should not have a NextAction set, something bad happened"));
	
	if(GetBrainComponent()->LogState && result.Change != ENLActionChangeType::NONE)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		UE_LOG(LogNextLife, Warning, TEXT("%s : %s:%s: "),
									  fromRequest ? TEXT("ApplyActionEventResponse") : TEXT("ApplyActionResult"),
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
					UE_LOG(LogNextLife, Warning, TEXT("%s CHANGE to %s : %s"), *Action->GetName(), 
																		  *result.Action->GetName(),
																		  *result.Reason);
					CLEAR_WARN_COLOR();
				}

				// Create the new action
				UNLAction* newAction = NewObject<UNLAction>(this, result.Action);
				check(newAction);

				// Swap to previous action while we invoke done (so events don't hit the ending action)
				UNLAction* oldAction = Action;
				Action = Action->PreviousAction;

				// End the current action
				oldAction->InvokeOnDone(newAction);

				// Commit the new action as head
				newAction->PreviousAction = Action;
				newAction->PreviousAction->NextAction = newAction;
				Action = newAction;

				// Start the new action and apply the result which could cause several actions to start via the recursion.
				const FNLActionResult newActionResult = Action->InvokeOnStart(result.Payload);
				return ApplyActionResult(newActionResult, fromRequest);
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
					UE_LOG(LogNextLife, Warning, TEXT("%s SUSPEND for %s : %s"), *Action->GetName(), 
																		    *result.Action->GetName(),
																		    *result.Reason);
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
					Action->PreviousAction->NextAction = Action;
				}
				else
				{
					// Swap to previous action while we invoke done (so events don't hit the ending action)
					UNLAction* oldAction = Action;
					Action = Action->PreviousAction;

					// End the current action
					oldAction->InvokeOnDone(newAction);

					// Commit the new action as head
					newAction->PreviousAction = Action;
					newAction->PreviousAction->NextAction = newAction;
					Action = newAction;
				}

				// Start the new action and apply the result which could cause several actions to start via the recursion.
				const FNLActionResult newActionResult = Action->InvokeOnStart(result.Payload);
				return ApplyActionResult(newActionResult, fromRequest);
			}
		case ENLActionChangeType::DONE:
			{
				if(GetBrainComponent()->LogState)
				{
					SET_WARN_COLOR(COLOR_RED);
					UE_LOG(LogNextLife, Warning, TEXT("%s DONE : %s"), *Action->GetName(), *result.Reason);
					CLEAR_WARN_COLOR();
				}

				UNLAction* endingAction = Action;
				Action = Action->PreviousAction;
				endingAction->InvokeOnDone(Action);

				if(Action)
				{
					// Resume the action and let it apply an action result
					Action->NextAction = nullptr;
					
					const FNLActionResult resumeResult = Action->InvokeOnResume(endingAction);
					return ApplyActionResult(resumeResult, fromRequest);
				}

				// No more actions, this behavior has completed!
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
			default:
				break;
		}
		SET_WARN_COLOR(COLOR_CYAN);
		UE_LOG(LogNextLife, Warning, TEXT("%s:%s %s EVENT '%s' with request %s - '%s'"), *GetName(),
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
	while(Action && !Action->EventResponse.IsNone())
	{
		// Create a new action from the event
		FNLActionResult newAction;
		CreateActionResultFromEvent(Action->EventResponse, newAction);

		// Clear now so if any other events occur from the action result they won't be affected
		Action->EventResponse = FNLEventResponse();

		// Apply the top level response immediately
		Action = ApplyActionResult(newAction, true);
	}

	if(!Action)
	{
		return Action;
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
		previousAction = previousAction->PreviousAction;
	}

	if(!requestedResponse.IsNone())
	{
		bool useNormalBehavior = true;
		if(requestedResponse.ChangeRequest == ENLActionChangeType::SUSPEND &&
			requestedResponse.SuspendBehavior != ENLSuspendBehavior::NORMAL)
		{
			if(requestedResponse.SuspendBehavior == ENLSuspendBehavior::TAKEOVER ||
			   requestedResponse.SuspendBehavior == ENLSuspendBehavior::TAKEOVER_APPEND)
			{
				// If this allows takeover, check the stack for an action which could take this over
				UNLAction* nextAction = Action;
				while(nextAction && nextAction != requestingAction)
				{
					if(nextAction->GetClass() == requestedResponse.Action)
					{
						if(nextAction->OnRequestTakeover(requestedResponse, requestingAction))
						{
							useNormalBehavior = false;
							
							// The takeover action has become the top action for now (this is so events from OnDone don't consider actions about to end)
							Action = nextAction;
				
							// The takeover action could be the current action, in which case, no extra action is required.
							if(Action->NextAction)
							{
								// Clear all actions above the takeover action
								Action->NextAction->InvokeOnDone(Action);
							
								// Resume the takeover action
								UNLAction* oldAction = Action->NextAction;
								Action->NextAction = nullptr;
								Action = ApplyActionResult(Action->InvokeOnResume(oldAction), true);
							}
							break;
						}
					}
					nextAction = nextAction->PreviousAction;
				}

				if(useNormalBehavior)
				{
					if(requestedResponse.SuspendBehavior == ENLSuspendBehavior::TAKEOVER_APPEND)
					{
						// Switch over to an append
						requestedResponse.SuspendBehavior = ENLSuspendBehavior::APPEND;
					}
					else
					{
						// Setup the event to be normal
						requestedResponse.SuspendBehavior = ENLSuspendBehavior::NORMAL;
					}
				}
			}

			if(requestedResponse.SuspendBehavior == ENLSuspendBehavior::APPEND)
			{
				// Appends don't backout onto normal behavior
				useNormalBehavior = false;
				
				// Suspend appends means we just want to put the action ontop of the top acton.
				// Request this of the top action, this suspend it if we can do it.
				if(Action->OnRequestEvent(requestedResponse, requestingAction))
				{
					// Now run the suspend normally
					FNLActionResult newAction;
					CreateActionResultFromEvent(requestedResponse, newAction);
					Action = ApplyActionResult(newAction, true);
				}
			}
		}

		if(useNormalBehavior)
		{
			// Now if this request is not None, request that this action go through from the top of the action stack down to the requester
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
				// The requesting action has become the top action for now (this is so events from OnDone don't consider actions about to end)
				Action = requestingAction;
				
				// Clear all actions above the requesting action
				check(Action->NextAction);
				Action->NextAction->InvokeOnDone(requestingAction);
				Action->NextAction = nullptr;

				// Now run the event
				FNLActionResult newAction;
				CreateActionResultFromEvent(requestedResponse, newAction);
				Action = ApplyActionResult(newAction, true);
			}
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
	if(!AreEventsPaused())
	{
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
	}
    return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sight_Implementation(APawn* subject, bool indirect)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
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
	}
	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_SightLost_Implementation(APawn* subject)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
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
	}
	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sound_Implementation(APawn* OtherActor, const FVector& Location, float Volume, int32 flags)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
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
	}
	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Contact_Implementation(AActor* other, const FHitResult& hitResult)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
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
	}
	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Movement_MoveTo_Implementation(const AActor* goal, const FVector& pos, float range)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
		UNLAction* curAction = Action;
		while(curAction)
		{
			if(curAction->Implements<UNLMovementEvents>())
			{
				responseOut = INLMovementEvents::Execute_Movement_MoveTo(curAction, goal, pos, range);
				if(HandleEventResponse(curAction, TEXT("Movement_MoveTo"), responseOut))
				{
					break;
				}
			}

			curAction = curAction->PreviousAction;
		}
	}
	return responseOut;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Movement_MoveToComplete_Implementation(FAIRequestID RequestID, const EPathFollowingResult::Type Result)
{
	FNLEventResponse responseOut;
	if(!AreEventsPaused())
	{
		UNLAction* curAction = Action;
		while(curAction)
		{
			if(curAction->Implements<UNLMovementEvents>())
			{
				responseOut = INLMovementEvents::Execute_Movement_MoveToComplete(curAction, RequestID, Result);
				if(HandleEventResponse(curAction, TEXT("Movement_MoveToComplete"), responseOut))
				{
					break;
				}
			}

			curAction = curAction->PreviousAction;
		}
	}
	return responseOut;
}
