// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "Engine.h"

// Declare an outside visible logging category so generated class code can use the ToLife log category from different modules.
NEXTLIFE_API DECLARE_LOG_CATEGORY_EXTERN(LogNextLife, Log, All);

#define NEXTLIFE_MODULE_NAME "NextLife"

DECLARE_STATS_GROUP(TEXT("NextLife"), STATGROUP_NextLife, STATCAT_Advanced);

//////////////////////////////////////////////////////////////////////////
// INextLifeModule

class INextLifeModule : public IModuleInterface
{
public:
    /**
    * Singleton-like access to this module's interface.  This is just for convenience!
    * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
    *
    * @return Returns singleton instance, loading the module on demand if needed
    */
    static FORCEINLINE INextLifeModule& Get()
    {
        return FModuleManager::LoadModuleChecked< INextLifeModule >(NEXTLIFE_MODULE_NAME);
    }

    /**
    * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
    *
    * @return True if the module is loaded and ready to use
    */
    static FORCEINLINE bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded(NEXTLIFE_MODULE_NAME);
    }
};
