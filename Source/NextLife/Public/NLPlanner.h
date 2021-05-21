// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLPlanner.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * A base planner used to plan things related to NextLife AIs
*/
UCLASS(Blueprintable, BlueprintType)
class NEXTLIFE_API UNLPlanner : public UObject
{
    GENERATED_BODY()
public:
    UNLPlanner();

    // Called when determining if a NextLife AI should be part of this plan or filtered out
    //UFUNCTION(BlueprintNativeEvent, Category="NextLife|Planner")
    //bool FilterForPlan(TScriptInterface<INextLife>& nextlifeToFilter);
    //virtual bool FilterForPlan_Implementation(TScriptInterface<INextLife>& nextlifeToFilter) { return false; }
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * Next Life Plan Coordinator
*/
UCLASS(Blueprintable, BlueprintType)
class NEXTLIFE_API ANLPlanCoordinator : public AActor
{
public:
    GENERATED_BODY()

    ANLPlanCoordinator();

    UFUNCTION(BlueprintCallable, Category = "NextLife|PlanCoordination")
    void RegisterPlanner(UNLPlanner* planner);
};
