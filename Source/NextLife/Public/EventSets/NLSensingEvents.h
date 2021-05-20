// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NextLife.h"
#include "NLSensingEvents.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLSensingEvents : public UInterface
{
    GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
 * General Events related to sensing
*/
class NEXTLIFE_API INLSensingEvents
{
public:
    GENERATED_BODY()

    // On sight of an interesting NextLife AI
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
    void Sense_Sight(TScriptInterface<INextLife>& subject);
    virtual void Sense_Sight_Implementation(TScriptInterface<INextLife>& subject) { }

    // On lost sight of an interesting NextLife AI
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
    void Sense_LostSight(TScriptInterface<INextLife>& subject);
    virtual void Sense_LostSight_Implementation(TScriptInterface<INextLife>& subject) { }

    // On hearing an interesting sound
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
    void Sense_Sound(const int32 soundFlags, TScriptInterface<INextLife>& source, const FVector& sourceLocation);
    virtual void Sense_Sound_Implementation(const int32 soundFlags, TScriptInterface<INextLife>& source, const FVector& sourceLocation) { }

    // On contact with a world surface or actor. Actor could be an INextLife implementer.
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|SensingEvents")
    void Sense_Contact(AActor* other, const FHitResult& hitResult);
    virtual void Sense_Contact_Implementation(AActor* other, const FHitResult& hitResult) { }
};
