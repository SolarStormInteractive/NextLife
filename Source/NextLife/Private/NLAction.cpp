// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NLAction.h"

#include "AIController.h"
#include "NLBehavior.h"
#include "NextLifeBrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction::UNLAction()
{

}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
APawn* UNLAction::GetPawnOwner() const
{
	AAIController* AIController = GetAIOwner();
	if(AIController)
	{
		return AIController->GetPawn();
	}

	return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
AAIController* UNLAction::GetAIOwner() const
{
	UNLBehavior* myBehavior = GetBehavior();
	if(myBehavior)
	{
		UNextLifeBrainComponent* brainComponent = myBehavior->GetBrainComponent();
		if(brainComponent)
		{
			return brainComponent->GetAIOwner();
		}
	}

	return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLBehavior* UNLAction::GetBehavior() const
{
	return Cast<UNLBehavior>(GetOuter());
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UBlackboardComponent* UNLAction::GetBlackboard() const
{
	AAIController* AIOwner = GetAIOwner();
	if(AIOwner)
	{
		return AIOwner->GetBlackboardComponent();
	}
	return nullptr;
}
