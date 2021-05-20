// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.


#include "NLBehavior.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UNLBehavior::UNLBehavior()
{

}

//---------------------------------------------------------------------------------------------------------------------
/**
*/
void UNLBehavior::UpdateBehavior(UObject* owningObject, float deltaSeconds)
{
    PreUpdateBehavior(owningObject, deltaSeconds);
    PostUpdateBehavior(owningObject, deltaSeconds);
}
