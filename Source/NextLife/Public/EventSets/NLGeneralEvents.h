﻿// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLTypes.h"
#include "NLGeneralEvents.generated.h"

// A generic message which can be sent to the AI.
// These are useful for game specific events about the AI such as "I was lit on fire"
// They derive from an action payload so they can be used for inputs as a message, but continue as a payload to a suspend.
UCLASS(BlueprintType, Blueprintable)
class NEXTLIFE_API UNLGeneralMessage : public UNLActionPayload
{
	GENERATED_BODY()
public:

	// The name of the message.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GenericMessage")
	FName MessageName;
};

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLGeneralEvents : public UInterface
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
* General Events for NextLife AIs
*/
class NEXTLIFE_API INLGeneralEvents
{
public:
	GENERATED_BODY()

	/**
	 * A General Message Event
	 * This can contain any information needed for the AI that is game specific and can cause changes to the behaviors
	 * action stack.
	*/
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|GeneralEvents")
	FNLEventResponse General_Message(UNLGeneralMessage* message);
	virtual FNLEventResponse General_Message_Implementation(UNLGeneralMessage* message) { return FNLEventResponse(); };
};
