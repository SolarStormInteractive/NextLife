// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "AIModule\Classes\BrainComponent.h"
#include "NextLifeBrainComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FNLBehaviorChooser, class UNLBehavior*, BehaviorToAssess);

USTRUCT(BlueprintType)
struct FNLBehaviorHolder
{
	GENERATED_BODY()
	FNLBehaviorHolder()
		: Behavior(nullptr)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Behavior Holder")
	class UNLBehavior* Behavior;

	UPROPERTY()
	FNLBehaviorChooser Chooser;
};

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

	// Should this brain automatically hook up events to behaviors for pawn sensing so events
	// such as sight, hearing, touch will fire when these events happen to the pawn?
	// It can be useful to handle these events manually so you can control the information being fed to the AI.
	// NOTE: For sight and hearing, the pawn must contain a PawnSensingComponent
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brain")
	bool AutoHookUpSenses;
	
	// Add a behavior to this brain
	UFUNCTION(BlueprintCallable, Category = "NextLife|Brain")
	bool AddBehavior(TSubclassOf<class UNLBehavior> behaviorClass, FNLBehaviorChooser chooser);

	// Remove a behavior from this brain
	UFUNCTION(BlueprintCallable, Category = "NextLife|Brain")
	bool Removebehavior(TSubclassOf<class UNLBehavior> behaviorClass);

	// Called when choosing behaviors to run this frame. By default, uses the supplied conditional delegate to determine which behaviors to run.
	// You can override this to control which behaviors are choosen to run with more complex logic.
	UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Brain")
	void ChooseBehaviors(TArray<class UNLBehavior*>& behaviorsOut);

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

protected:

	UPROPERTY(BlueprintReadOnly, Category = "NextLife|Brain")
	TArray<FNLBehaviorHolder> Behaviors;

	UPROPERTY()
	TArray<TSubclassOf<class UNLBehavior>> ActiveBehaviorClasses;

	UPROPERTY()
	bool AreBehaviorsPaused;

	UPROPERTY()
	bool LogicIsStarted;
};
