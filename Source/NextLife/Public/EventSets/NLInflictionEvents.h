// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

#pragma once

//#include "NextLife.h"
#include "NLInflictionEvents.generated.h"

/*UENUM(BlueprintType, DisplayName = "NextLife Damage Type")
enum class ENLDamageType : uint8
{
    // Damage from a point
    DAMAGE_TYPE_POINT,
    // Damage from a radial source ( grenade, AOE)
    DAMAGE_TYPE_RADIAL,
    // Damage directly dealt but with no world source.
    DAMAGE_TYPE_NO_SOURCE,
};

USTRUCT(BlueprintType, DisplayName = "NextLife Damage Definition")
struct FNLDamageDefinition
{
public:
    GENERATED_USTRUCT_BODY()

	FNLDamageDefinition() :
          DamageType(ENLDamageType::DAMAGE_TYPE_NO_SOURCE)
        , Damage1(0), Damage2(0), Damage3(0), Damage4(0)
        , DamageOrigin(FVector::ZeroVector)
        , DamagePosition(FVector::ZeroVector)
        , Attacker(nullptr)
        , DamageCauser(nullptr)
        , DamageFlags(0)
        , forceDir(FVector::ZeroVector)
        , force(0)
        , boneName(NAME_None)
        , ImpactNormal(FVector::ZeroVector)
	{
	}

	// The type of damage at the area of the origin
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	ENLDamageType DamageType;

	// Primary Damage value
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
    float Damage1;

	// Damage 2 value
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
    float Damage2;

    // Damage 3 value
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
    float Damage3;

    // Damage 4 value
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
    float Damage4;

	// Where did the damage come from?
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	FVector DamageOrigin;

	// Where were we hit?
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	FVector DamagePosition;

	// The attacker pawn
	UPROPERTY(BlueprintReadWrite, SkipSerialization, Category = "NextLife|DamageDefinition")
	TScriptInterface<INextLife> Attacker;

	// The damage causer, bullet or something
	UPROPERTY(BlueprintReadWrite, SkipSerialization, Category = "NextLife|DamageDefinition")
	TScriptInterface<INextLife> DamageCauser;

	// An extra bitfield for game specific damage flags
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	int32 DamageFlags;

	// The force direction
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	FVector forceDir;

	// The force multiplier
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	float force;
	
	// The bone name to apply force to
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	FName boneName;

	// Normal for hit
	UPROPERTY(BlueprintReadWrite, Category = "NextLife|DamageDefinition")
	FVector ImpactNormal;
};*/


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
 * General Events related to infliction
*/
class NEXTLIFE_API INLInflictionEvents
{
public:
    GENERATED_BODY()

	
    /*
    // Events for infliction upon a NextLife AI
    UFUNCTION(BlueprintNativeEvent, Category="NextLife|InflictionEvents")
    void Infliction_Injured(TScriptInterface<INextLife>& inflictor, const FNLDamageDefinition& damageDefinition);
    virtual void Infliction_Injured_Implementation(TScriptInterface<INextLife>& inflictor, const FNLDamageDefinition& damageDefinition) {}
    */
};
