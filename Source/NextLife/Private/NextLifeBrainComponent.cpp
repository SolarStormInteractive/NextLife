// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NextLifeBrainComponent.h"
#include "NextLifeModule.h"
#include "NLBehavior.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNextLifeBrainComponent::UNextLifeBrainComponent()
	: AutoHookUpSenses(true)
	, LogState(false)
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
		UNLBehavior* newBehavior;
		{
			FNLBehaviorHolder newHolder;
			newBehavior = NewObject<UNLBehavior>(this, behaviorClass);
			newHolder.Behavior = newBehavior;
			newHolder.Chooser = chooser;
			Behaviors.Add(newHolder);
			ActiveBehaviorClasses.Add(behaviorClass);
			newBehavior->OnBehaviorEnded.AddDynamic(this, &UNextLifeBrainComponent::OnBehaviorComplete);
		}
		if(newBehavior && LogicIsStarted)
		{
			// Beginning could cause an end.
			newBehavior->BeginBehavior();
		}
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::RemoveBehavior(TSubclassOf<UNLBehavior> behaviorClass)
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
		// Need to iterate backwards because starting behaviors could end them
		for(int32 behaviorIndex = Behaviors.Num() - 1; behaviorIndex >= 0; --behaviorIndex)
		{
			if(Behaviors[behaviorIndex].Behavior)
			{
				Behaviors[behaviorIndex].Behavior->BeginBehavior();
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::RestartLogic()
{
	if(LogicIsStarted)
	{
		// End the current behaviors and store the info to restart them
		TArray<TSubclassOf<UNLBehavior>> behaviorsToRestart;
		TArray<FNLBehaviorChooser> choosers;
		for(int32 behaviorIndex = Behaviors.Num() - 1; behaviorIndex >= 0; --behaviorIndex)
		{
			if(Behaviors[behaviorIndex].Behavior)
			{
				behaviorsToRestart.Add(Behaviors[behaviorIndex].Behavior->GetClass());
				choosers.Add(Behaviors[behaviorIndex].Chooser);
				Behaviors[behaviorIndex].Behavior->EndBehavior();
			}
		}

		// Restart the behaviors which were ended
		for(int32 behaviorIndex = 0; behaviorIndex < behaviorsToRestart.Num(); ++behaviorIndex)
		{
			AddBehavior(behaviorsToRestart[behaviorIndex], choosers[behaviorIndex]);
		}
	}
	else
	{
		// Was never started so just start it
		StartLogic();
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::StopLogic(const FString& Reason)
{
	if(LogicIsStarted)
	{
		for(int32 behaviorIndex = Behaviors.Num() - 1; behaviorIndex >= 0; --behaviorIndex)
		{
			if(Behaviors[behaviorIndex].Behavior)
			{
				Behaviors[behaviorIndex].Behavior->EndBehavior();
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

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::OnBehaviorComplete(UNLBehavior* completeBehavior)
{
	check(completeBehavior);
	
	const int32 classIndex = ActiveBehaviorClasses.Find(completeBehavior->GetClass());
	if(classIndex != INDEX_NONE)
	{
		ActiveBehaviorClasses.RemoveAt(classIndex);
		check(Behaviors.IsValidIndex(classIndex));
		check(Behaviors[classIndex].Behavior);
		Behaviors.RemoveAt(classIndex);
	}
}
