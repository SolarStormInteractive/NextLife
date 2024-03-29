// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NLAction.h"
#include "NextLifeModule.h"
#include "AIController.h"
#include "NLBehavior.h"
#include "NextLifeBrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLAction::UNLAction()
	: HasStarted(false)
{

}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLActionResult UNLAction::InvokeOnStart(UNLActionPayload* payload)
{
	HasStarted = true;
	return OnStart(payload);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLActionResult UNLAction::InvokeUpdate(float deltaSeconds)
{
	checkf(HasStarted, TEXT("Invoking an update on an action which has no started?"));
	return OnUpdate(deltaSeconds);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNLAction::InvokeOnSuspend(const UNLAction *interruptingAction)
{
	checkf(!NextAction, TEXT("Suspending an already suspended action?"));
	return OnSuspend(interruptingAction);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
FNLActionResult UNLAction::InvokeOnResume(const UNLAction *resumingFrom)
{
	return OnResume(resumingFrom);
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLAction::InvokeOnDone(const UNLAction* nextAction)
{
	OnDone(nextAction);
	if(NextAction)
	{
		NextAction->InvokeOnDone(nextAction);
		NextAction = nullptr;
	}
	PreviousAction = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
APawn* UNLAction::GetPawnOwner() const
{
	const AAIController* AIController = GetAIOwner();
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
	const UNLBehavior* myBehavior = GetBehavior();
	if(myBehavior)
	{
		const UNextLifeBrainComponent* brainComponent = myBehavior->GetBrainComponent();
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
float UNLAction::GetWorldTimeSeconds() const
{
	const AAIController* AIController = GetAIOwner();
	if(AIController)
	{
		return AIController->GetWorld()->GetTimeSeconds();
	}

	return -1.0f;
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
