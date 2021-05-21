// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NextLifeBrainComponent.h"
#include "NLBehavior.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNextLifeBrainComponent::UNextLifeBrainComponent()
	: AutoHookUpSenses(true)
	, AreBehaviorsPaused(false)
	, LogicIsStarted(false)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::AddBehavior(TSubclassOf<UNLBehavior> behaviorClass, FNLBehaviorChooser chooser)
{
	if(!ActiveBehaviorClasses.Contains(behaviorClass))
	{
		FNLBehaviorHolder newHolder;
		UNLBehavior* newBehavior = NewObject<UNLBehavior>(this, behaviorClass);
		newHolder.Behavior = newBehavior;
		newHolder.Chooser = chooser;
		Behaviors.Add(newHolder);
		ActiveBehaviorClasses.Add(behaviorClass);
		if(LogicIsStarted)
		{
			newBehavior->BeginBehavior();
		}
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::Removebehavior(TSubclassOf<UNLBehavior> behaviorClass)
{
	const int32 classIndex = ActiveBehaviorClasses.Find(behaviorClass);
	if(classIndex != INDEX_NONE)
	{
		ActiveBehaviorClasses.RemoveAt(classIndex);
		check(Behaviors.IsValidIndex(classIndex));
		check(Behaviors[classIndex].Behavior);
		if(LogicIsStarted)
		{
			Behaviors[classIndex].Behavior->EndBehavior();
		}
		Behaviors.RemoveAt(classIndex);
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::ChooseBehaviors_Implementation(TArray<class UNLBehavior*>& behaviorsOut)
{
	for(const FNLBehaviorHolder& holder : Behaviors)
	{
		if(!holder.Chooser.IsBound() || holder.Chooser.Execute(holder.Behavior))
		{
			behaviorsOut.Add(holder.Behavior);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::StartLogic()
{
	if(!LogicIsStarted)
	{
		LogicIsStarted = true;
		for(const FNLBehaviorHolder& holder : Behaviors)
		{
			if(holder.Behavior)
			{
				holder.Behavior->BeginBehavior();
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::RestartLogic()
{
	for(const FNLBehaviorHolder& holder : Behaviors)
	{
		if(holder.Behavior)
		{
			if(LogicIsStarted)
			{
				holder.Behavior->EndBehavior();
			}
			holder.Behavior->BeginBehavior();
		}
	}
	LogicIsStarted = true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::StopLogic(const FString& Reason)
{
	if(LogicIsStarted)
	{
		for(const FNLBehaviorHolder& holder : Behaviors)
		{
			if(holder.Behavior)
			{
				holder.Behavior->EndBehavior();
			}
		}
		LogicIsStarted = false;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Cleanup()
{
	if(LogicIsStarted)
	{
		for(const FNLBehaviorHolder& holder : Behaviors)
		{
			if(holder.Behavior)
			{
				holder.Behavior->EndBehavior();
			}
		}
		LogicIsStarted = false;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::PauseLogic(const FString& Reason)
{
	AreBehaviorsPaused = true;
	for(const FNLBehaviorHolder& holder : Behaviors)
	{
		if(holder.Behavior)
		{
			holder.Behavior->SetEventsPausedState(true);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
EAILogicResuming::Type UNextLifeBrainComponent::ResumeLogic(const FString& Reason)
{
	AreBehaviorsPaused = false;
	for(const FNLBehaviorHolder& holder : Behaviors)
	{
		if(holder.Behavior)
		{
			holder.Behavior->SetEventsPausedState(false);
		}
	}
	return EAILogicResuming::Continue;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::IsRunning() const
{
	return LogicIsStarted && !AreBehaviorsPaused;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::IsPaused() const
{
	return AreBehaviorsPaused;
}
