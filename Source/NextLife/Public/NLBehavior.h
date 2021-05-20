// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLAction.h"
#include "NLBehavior.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Behavior : The base behavior of a NextLife AI. This should be extended to fit the behavior of the AI you are trying to create.
 * See example behaviors in the Behaviors folder.
*/
UCLASS(Blueprintable)
class NEXTLIFE_API UNLBehavior : public UObject
{
    GENERATED_BODY()
public:
    UNLBehavior();

    /**
     * Called when this behavior begins.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Behavior")
    void BeginBehavior();
    virtual void BeginBehavior_Implementation() {}

    /**
     * Called when this behavior is about to end.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Behavior")
    void EndBehavior();
    virtual void EndBehavior_Implementation() {}

    /**
     * Called when this behavior should update its current action state.
    */
    void UpdateBehavior(UObject* owningObject, float deltaSeconds);

    /**
     * Called when this behavior is about to update, possibly changing the current action state.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Behavior")
    void PreUpdateBehavior(UObject* owningObject, float deltaSeconds);
    virtual void PreUpdateBehavior_Implementation(UObject* owningObject, float deltaSeconds) {}

    /**
     * Called when this behavior has updated and possibly changed the current action state.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "NextLife|Behavior")
    void PostUpdateBehavior(UObject* owningObject, float deltaSeconds);
    virtual void PostUpdateBehavior_Implementation(UObject* owningObject, float deltaSeconds) {}
};
