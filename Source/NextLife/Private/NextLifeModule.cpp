// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#include "NextLifeModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogNextLife);

#define LOCTEXT_NAMESPACE "NextLife"

class FNextLifeModule : public INextLifeModule
{
public:
    FNextLifeModule()
    {
    }

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FNextLifeModule, NextLife);

//////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------------------------
/**
*/
void FNextLifeModule::StartupModule()
{
}

//--------------------------------------------------------------------------------------------------------------------
/**
*/
void FNextLifeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
