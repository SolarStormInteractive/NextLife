// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "Actions/Humanoid/NLHumanoidIdle.h"
#include "NextLifeModule.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLHumanoidIdle::Sense_Sight_Implementation(APawn* subject, bool indirect)
{
	if(subject)
	{
		UE_LOG(LogNextLife, Log, TEXT("I notice a pawn named %s!"), *subject->GetName())
	}

	return TryContinue();
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLEventResponse UNLHumanoidIdle::Sense_SightLost_Implementation(APawn* subject)
{
	if(subject)
	{
		UE_LOG(LogNextLife, Log, TEXT("A pawn named %s just ran off!"), *subject->GetName())
	}

	return TryContinue();
}
