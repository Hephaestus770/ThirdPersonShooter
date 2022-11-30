// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Drone.generated.h"

UCLASS()
class SHOOTERPROJESI_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value); // Set MoveForwardValue to the Value and calls DroneMovement() function

	void MoveRight(float Value); //  Set MoveRightValue to the Value and calls DroneMovement() function

	void MoveUp(float Value); //  Set MoveUpValue to the Value and calls DroneMovement() function

	void DroneMovement(); // Update actor's velocity based on MoveForwardValue, MoveRightValue, MoveUpValue

	void UpdateSpringArm(); // Rotate spring arm component

	void RotateCameraFocus(); // Rotate camera to drone

	void UpdateCameraFOV(); // Update camera fiel of view based on drone's velocity

	void DroneDash(); // Small dash based on drone's velocity 

	void SetBoostState(); //  Set bIsBoostReady for cooldown

	void Fire(); 

	bool GetBeamEndLocation(const FVector& DroneSocketLocation, FVector& EndLocation);


	void Turn(float Value);

	void LookUp(float Value);

	// These can be used with arrow keys but are specifically for Gamepad sticks.
	void LookUpAtRate(float Rate);
	void TurnAtRate(float Rate);





public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* DroneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MoveForwardValue; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MoveRightValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MoveUpValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float TurnValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LookUpValue;

	UPROPERTY(VisibleAnywhere, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MovementSpeed; // Movement speed for drone

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CameraSpeed; // Camera speed for drone

	bool bIsBoostReady;
	
	FTimerHandle BoostTimer;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	class USoundCue* FireSound;


};
