// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPROJESI_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

		UFUNCTION(BlueprintCallable)
		void UpdateAnimationProperties(float DeltaTime); // this is tick() funticon for animation

		virtual void NativeInitializeAnimation() override; // this is like beginplay() function for animation

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess="true"))
	class AShooterCharacter* ShooterCharacter;

	// Character's speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta =(AllowPrivateAccess="true"))
	float Speed;

	// Is Character in air
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	// Is Character moving
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	// OffSet Yaw used for strafing 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MovementOffSetYaw;

	// Offset yaw value before the character stop 
	/*
		We need this value for the stop animation blend space 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LastMovementOffSetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess= "true"))
	bool bAiming;

};
