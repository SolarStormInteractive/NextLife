// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "AIModule\Classes\BrainComponent.h"

#include "EventSets/NLGeneralEvents.h"
#include "EventSets/NLMovementEvents.h"

#include "NextLifeBrainComponent.generated.h"

/**
 * NextLife Brain Component
 * To use a NextLife style brain for your AI Controller
 */
UCLASS()
class NEXTLIFE_API UNextLifeBrainComponent : public UBrainComponent
{
	GENERATED_BODY()
public:
	UNextLifeBrainComponent();

	// If true, all behavior state will be logged. Actions starting, updating, changing, suspending, ending, etc...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brain")
	bool LogState;
	
	// Add a behavior to this brain
	UFUNCTION(BlueprintCallable, Category = "NextLife|Brain")
	bool AddBehavior(TSubclassOf<class UNLBehavior> behaviorClass);

	// Remove a behavior from this brain
	UFUNCTION(BlueprintCallable, Category = "NextLife|Brain")
	bool RemoveBehavior(TSubclassOf<class UNLBehavior> behaviorClass);

	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Brain")
	bool ShouldChooseBehavior(class UNLBehavior* behaviorToAssess);
	virtual bool ShouldChooseBehavior_Implementation(class UNLBehavior* behaviorToAssess) { return true; }

	// Ticks all behaviors currently active
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/** Starts brain logic. If brain is already running, will not do anything. */
	virtual void StartLogic() override;

	/** Restarts currently running or previously ran brain logic. */
	virtual void RestartLogic() override;

	/** Stops currently running brain logic. */
	virtual void StopLogic(const FString& Reason) override;

	/** AI logic won't be needed anymore, stop all activity and run cleanup */
	virtual void Cleanup() override;

	/** Pause logic and blackboard updates. */
	virtual void PauseLogic(const FString& Reason) override;
	
	/** Resumes paused brain logic.
	*  MUST be called by child implementations!
	*	@return indicates whether child class' ResumeLogic should be called (true) or has it been 
	*	handled in a different way and no other actions are required (false)*/
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;

	virtual bool IsRunning() const override;
	virtual bool IsPaused() const override;

	/**
	* INLGeneralEvents Implementation
	*/
	void General_Message(UNLGeneralMessage* message);
	
	/**
	* INLSensingEvents Implementation
	*/
	void Sense_Sight(APawn* subject, bool indirect);
	void Sense_SightLost(APawn* subject);
	void Sense_Sound(APawn *OtherActor, const FVector &Location, float Volume, int32 flags);
	void Sense_Contact(AActor* other, const FHitResult& hitResult);

	/**
	* INLMovementEvents Implementation
	*/
	void Movement_MoveTo(const AActor *goal, const FVector &pos, float range);
	void Movement_MoveToComplete(FAIRequestID RequestID, const EPathFollowingResult::Type Result);
	
protected:

	// Called when choosing behaviors to run this frame. By default, uses the supplied conditional delegate to determine which behaviors to run.
	// You can override this to control which behaviors are choosen to run with more complex logic.
	void ChooseBehaviors(TArray<int32>& behaviorsOut);

	UFUNCTION()
	void OnBehaviorComplete(class UNLBehavior* completeBehavior);
	
	UPROPERTY(BlueprintReadOnly, Category = "NextLife|Brain", SkipSerialization)
	TArray<class UNLBehavior*> Behaviors;
	
	UPROPERTY(SkipSerialization)
	TArray<TSubclassOf<class UNLBehavior>> ActiveBehaviorClasses;

	UPROPERTY(SaveGame)
	bool AreBehaviorsPaused;

	UPROPERTY(SaveGame)
	bool LogicIsStarted;
};
