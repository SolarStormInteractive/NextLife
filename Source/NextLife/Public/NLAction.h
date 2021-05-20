// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLAction.generated.h"

USTRUCT()
struct FNLActionResult
{
    GENERATED_BODY()
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * Base Action : TODO WRITE DOCS
*/
UCLASS(Blueprintable)
class NEXTLIFE_API UNLAction : public UObject
{
    GENERATED_BODY()
public:
    UNLAction();
};
