// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NLBehavior.h"
#include "NextLifeBrainComponent.h"
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
UNextLifeBrainComponent* UNLBehavior::GetBrainComponent() const
{
	return Cast<UNextLifeBrainComponent>(GetOuter());
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::RunBehavior(float deltaSeconds)
{
	
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
	if(Action)
    {
    	if(Action->Implements<UNLSensingEvents>())
    	{
    		const FNLEventResponse response = INLGeneralEvents::Execute_General_Message(Action, message);
    		if(response.Request != ENLEventRequest::NONE)
    		{
    			
    		}
    	}
    }

    return FNLEventResponse();
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sight_Implementation(APawn* subject, bool indirect)
{
	return FNLEventResponse();
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_SightLost_Implementation(APawn* subject)
{
	return FNLEventResponse();
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLBehavior::Sense_Sound_Implementation(APawn* OtherActor, const FVector& Location, float Volume, int32 flags)
{
	return FNLEventResponse();
}
