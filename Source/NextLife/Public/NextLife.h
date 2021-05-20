// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLBehavior.h"
#include "NextLife.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE(BlueprintType)
class NEXTLIFE_API UNextLife : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
 * Implement this interface into any UObject which wishes to execute NextLife behavioral logic on itself
 * NOTE: This is a blueprintable system.
 * Rememeber to implement BlueprintNativeEvent in c++ code using the {signature_name}_implementation({}) syntax.
*/
class NEXTLIFE_API INextLife
{
public:
    GENERATED_IINTERFACE_BODY()

    /**
     * Implementation of returning the next behavior.
     * @param currentBehavior: If nullptr, get the first important behavior. If non-nullptr, get the next behavior after currentBehavior
     * return: The next behavior to evaluate. nullptr if no further behaviors should be evaluated.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "NextLife")
    UNLBehavior* GetNextBehavior(UNLBehavior* currentBehavior = nullptr);
};
