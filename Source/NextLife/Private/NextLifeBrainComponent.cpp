// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NextLifeBrainComponent.h"
#include "NextLifeModule.h"
#include "NLBehavior.h"

#include "AIController.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNextLifeBrainComponent::UNextLifeBrainComponent()
	: LogState(false)
	, AreBehaviorsPaused(false)
	, LogicIsStarted(false)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
bool UNextLifeBrainComponent::AddBehavior(TSubclassOf<UNLBehavior> behaviorClass)
{
	if(!ActiveBehaviorClasses.Contains(behaviorClass))
	{
		UNLBehavior* newBehavior = NewObject<UNLBehavior>(this, behaviorClass);
		Behaviors.Add(newBehavior);
		ActiveBehaviorClasses.Add(behaviorClass);
		newBehavior->OnBehaviorEnded.AddDynamic(this, &UNextLifeBrainComponent::OnBehaviorComplete);
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
		check(Behaviors[classIndex]);
		if(LogicIsStarted)
		{
			Behaviors[classIndex]->StopBehavior(true);
		}
		Behaviors.RemoveAt(classIndex);
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::ChooseBehaviors(TArray<int32>& behaviorsOut)
{
	int32 behaviorIndex = 0;
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(ShouldChooseBehavior(behavior))
		{
			behaviorsOut.Add(behaviorIndex);
		}
		++behaviorIndex;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<int32> behaviorsToRun;
	ChooseBehaviors(behaviorsToRun);

	// Stop any behaviors which shouldn't be running right now first
	for(int32 behaviorIndex = Behaviors.Num() - 1; behaviorIndex >= 0; --behaviorIndex)
	{
		check(Behaviors[behaviorIndex]);
		if(!behaviorsToRun.Contains(behaviorIndex))
		{
			// If behavior was running, we should reset it
			if(Behaviors[behaviorIndex]->HasBehaviorBegun())
			{
				Behaviors[behaviorIndex]->StopBehavior(false);
			}
		}
	}

	// Run behaviors which should be active
	for(int32 behaviorIndex = Behaviors.Num() - 1; behaviorIndex >= 0; --behaviorIndex)
	{
		check(Behaviors[behaviorIndex]);
		if(behaviorsToRun.Contains(behaviorIndex))
		{
			if(!Behaviors[behaviorIndex]->HasBehaviorBegun())
			{
				// Start it up
				Behaviors[behaviorIndex]->BeginBehavior();
			}
			else
			{
				// Run it
				Behaviors[behaviorIndex]->RunBehavior(DeltaTime);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::StartLogic()
{
	LogicIsStarted = true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::RestartLogic()
{
	StopLogic(TEXT("Restarting Logic"));
	StartLogic();
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
			if(Behaviors[behaviorIndex])
			{
				Behaviors[behaviorIndex]->StopBehavior(true);
			}
		}
		LogicIsStarted = false;

		if(LogState)
		{
			FString aiName = TEXT("Unknown");
			if(GetAIOwner())
			{
				if(GetAIOwner()->GetPawn())
				{
					aiName = GetAIOwner()->GetPawn()->GetName();
				}
				else
				{
					aiName = GetAIOwner()->GetName();
				}
			}
			UE_LOG(LogNextLife, Warning, TEXT("AI '%s' Logic being stopped, reason: %s"), *aiName, *Reason);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Cleanup()
{
	StopLogic(TEXT("Normal Cleanup"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::PauseLogic(const FString& Reason)
{
	AreBehaviorsPaused = true;
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior)
		{
			behavior->SetEventsPausedState(true);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
EAILogicResuming::Type UNextLifeBrainComponent::ResumeLogic(const FString& Reason)
{
	AreBehaviorsPaused = false;
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior)
		{
			behavior->SetEventsPausedState(false);
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
		check(Behaviors[classIndex]);
		Behaviors.RemoveAt(classIndex);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::General_Message(UNLGeneralMessage* message)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_General_Message(behavior, message);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Sense_Sight(APawn* subject, bool indirect)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Sense_Sight(behavior, subject, indirect);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Sense_SightLost(APawn* subject)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Sense_SightLost(behavior, subject);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Sense_Sound(APawn* OtherActor, const FVector& Location,
	float Volume, int32 flags)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Sense_Sound(behavior, OtherActor, Location, Volume, flags);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Sense_Contact(AActor* other, const FHitResult& hitResult)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Sense_Contact(behavior, other, hitResult);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Movement_MoveTo(const AActor* goal, const FVector& pos, float range)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Movement_MoveTo(behavior, goal, pos, range);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNextLifeBrainComponent::Movement_MoveToComplete(FAIRequestID RequestID, const EPathFollowingResult::Type Result)
{
	for(UNLBehavior*& behavior : Behaviors)
	{
		if(behavior && behavior->HasBehaviorBegun())
		{
			behavior->Execute_Movement_MoveToComplete(behavior, RequestID, Result);
		}
	}
}
