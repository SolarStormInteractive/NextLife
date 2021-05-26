// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

#include "NLInflictionEvents.generated.h"

//---------------------------------------------------------------------------------------------------------------------
/**
*/
UINTERFACE()
class NEXTLIFE_API UNLInflictionEvents : public UInterface
{
    GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------------------
/**
 * General Events related to infliction (taking damage)
*/
class NEXTLIFE_API INLInflictionEvents
{
public:
    GENERATED_BODY()

	/** General Take Damage Event, like Actors generically receive */
	UFUNCTION(BlueprintNativeEvent, Category="NextLife|InflictionEvents")
	FNLEventResponse Infliction_TakeDamage(const float Damage,
										   struct FDamageEvent const& DamageEvent,
										   const AController* EventInstigator,
										   const AActor* DamageCauser);
	virtual FNLEventResponse Infliction_TakeDamage_Implementation(const float Damage,
																  struct FDamageEvent const& DamageEvent,
																  const AController* EventInstigator,
																  const AActor* DamageCauser)
	{ return FNLEventResponse(); };
};
