// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTERPROJESI_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called for forward/backward input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);

	/*
		Called via input to look up/down at a given rate
		@param Rate This is a normalized rate, i.e. 1.0 means 100% of desired rate
	*/
	void LookUpAtRate(float Rate);

	/*
		Called via input to turn at a given rate
		@param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	// Change the sensivity of mouse APawn::AddControllerYawInput APawn::AddControllerPitchInput
	void Turn(float Value);
	void LookUp(float Value);

	//Called when FireButton is pressed
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	// Set bAiming to true or false
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float Deltatime);

	// Set BaseTurnRate and BaseLookUpRate based on aiming
	void SetLookRates();

	// Dash Ability
	void DashAbility();


	void SetDashActiveToFalse();

	void CalculateCrossHairSpread(float DeltaTime); // Calculate Cross hair spread based on character's movement

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	// Set bSwitchToAuto to true or false
	void SwitchBetweenShootingModes();

	// Switches camera side 
	void SwitchCameraSides();

	// SlowMotionAbility
	void SlowMotionAbility();

	// Spawn a Drone and posses it
	void DroneAbility();



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void DroneToPlayer();

private:
	
	/* Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	class USpringArmComponent* CameraBoom;

	/* Camera that follows character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float BaseLookUpRate;

	// Turn rate while not aiming
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float HipTurnRate;

	// Look up rate while not aiming
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float HipLookUpRate;

	// Turn rate while aiming
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float AimingTurnRate;

	// Look up rate while aiming
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"))
	float AimingLookUpRate;

	// Scale factor for mouse turn sensitivy not Aiminig
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"), meta = (ClampMin = "0.0", ClampMax = 1.0, UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	// Scale factor for mouse look sensitivy not Aiminig
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"), meta = (ClampMin = "0.0", ClampMax = 1.0, UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	// Scale factor for mouse turn sensitivy while Aiminig
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"), meta = (ClampMin = "0.0", ClampMax = 1.0, UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	// Scale factor for mouse look sensitivy while Aiminig
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "True"), meta = (ClampMin = "0.0", ClampMax = 1.0, UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	// Randomized 6 shot sound cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess="True"))
	class USoundCue* FireSound;
	
	// Flash spawned at BarrelSocket when gun fired
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	class UParticleSystem* MuzzleFlash;
	
	// Montage for firing weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	class UAnimMontage* HipFireMontage;

	// Particle for bullet impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	UParticleSystem* ImpactParticle;

	// Smoke trail for bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	UParticleSystem* BeamParticles;

	// Is Character aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	// Default camera field of view value
	float CameraDefaultFOV;
	// Camera zoomed field of view value
	float CameraZoomedFOV;
	// Current camera field of view
	float CameraCurrentFOV;
	
	// Interp speed for aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	// Is Dash ability available? 
	bool bIsDashActive;

	
	// Timer handle for dash cooldown
	FTimerHandle DashTimer;
	
	// Dash cooldown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Dash", meta = (AllowPrivateAccess = "true"))
	float DashCooldown;
	
	// Force multiplier for Dash ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Dash", meta = (AllowPrivateAccess = "true"))
	float ForceMultiplier;

	
	// Determines the spread of the crosshairs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;
	
	// Velocity component for crosshairs spread
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	// In air component for crosshairs spread
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	// Aim component for crosshair spread
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	// Shooting component for crosshair spread
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	// Time duration for crosshair set timer
	float ShootTimeDuraiton;

	// Is character shooting?
	bool bFiringBullet;

	// Timer handle for crosshair shoot spread
	FTimerHandle CrosshairShootTimer;

	// Left mouse button or right gamepad trigger pressed
	bool bFireButtonPressed;

	// True when we can fire, false when waiting for timer
	bool bShouldFire;

	// Fire rate of rifle
	float AutomaticFireRate;

	//	Sets a timer between gunshots
	FTimerHandle AutoFireTimerHandle;

	// Switch between singular shots to automatic 
	bool bSwitchToAuto;

	// Camera Y off set value
	float CameraYOffset;
		
	bool bSlowMoActive;


	// Drone Time
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	FTimerHandle DroneTimerHandle;
	

	// Drone control duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone Ability", meta = (AllowPrivateAccess = "true"))
	float DroneTime;

	// Drone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	TSubclassOf<APawn> Drone;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class ADrone* MyDrone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	USoundCue* DashSound;
	
	// Sound effects for the time slow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	USoundCue* SlowMoBeginSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	USoundCue* SlowMoEndSound;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "True"))
	USoundWave* SwitchModeSound;

public:
	/* Returns CameraBoom subobject */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/* Returns FollowCamera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }


	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

};
