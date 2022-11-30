// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Drone.h"
#include "TimerManager.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
	
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Initialize Camera Y offset value. This must be initialized before Camera->SocketOffset!
	CameraYOffset = 50.f;

	//Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f; // the camera follows at the distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, CameraYOffset, 70.f);
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of the boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Preventing the character to rotate when controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...with this rotation rate

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.3f;

	//Initialize turn rate and look up
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Turn rates for aiming/not aiming 
	HipTurnRate = 90.f;
	HipLookUpRate = 90.f;
	AimingTurnRate = 20.f;
	AimingLookUpRate = 20.f;

	// Mouse sensivity scale factors
	MouseHipTurnRate = 1.f;
	MouseHipLookUpRate = 1.f;
	MouseAimingTurnRate = .2f;
	MouseAimingLookUpRate = .2f;

	// Initialize Camera settings
	bAiming = false;
	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 50.f;
	ZoomInterpSpeed = 20.f;

	//Dash
	ForceMultiplier = 6.5f;
	bIsDashActive = false;
	DashCooldown = 3.0f;

	// Crosshair spread factors
	CrosshairSpreadMultiplier = 0.f;
	CrosshairVelocityFactor = 0.f;
	CrosshairInAirFactor = 0.f;
	CrosshairAimFactor = 0.f;
	CrosshairShootingFactor = 0.f;

	// Bullet fire timer variables
	ShootTimeDuraiton = 0.05f;
	bFiringBullet = false;

	// Auto rifle fire variables
	AutomaticFireRate = 0.1f;
	bFireButtonPressed = false;
	bShouldFire = true;
	bSwitchToAuto = false; // Switch between firing modes

	bSlowMoActive = false;

	// Drone control duration
	DroneTime = 3.f;


}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{	
		// Find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); //BaseLookUpRate = degree/second, DeltaTime = second/frame

}


void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); //BaseTurnRate = degree/second, DeltaTime = second/frame
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor = 0.f;
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor); 
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaelFactor = 0.f;
	if (bAiming)
	{
		LookUpScaelFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaelFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaelFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			// Spawn impact particles after updating BeamEndPoint
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEnd);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}

			}
		}

	}
	// Recoil Animation 
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire")); 
	}
	// Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
	 
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	
	// Get screen space of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;


	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld) // was deprojection succesfull?
	{
		FHitResult ScreenTraceHit;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = Start + CrosshairWorldDirection * 50'000.f;

		// Set beam end point to line trace end point
		OutBeamLocation = End;

		// Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit) // did trace hit?
		{
			// Beam end point is now trace hit location
			OutBeamLocation = ScreenTraceHit.Location;
			
			// This is for the testing, damage system is not complete
			UGameplayStatics::ApplyDamage(ScreenTraceHit.GetActor(), 1.f, nullptr, nullptr, nullptr);


			// Second trace from gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart = MuzzleSocketLocation;
			const FVector WeaponTraceEnd = OutBeamLocation;
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

			if (WeaponTraceHit.bBlockingHit)
			{																							
				OutBeamLocation = WeaponTraceHit.Location;
			}
			return true;
		}
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float Deltatime)
{
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, Deltatime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, Deltatime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

// Dash ability 
// This can be re-done with using Apply Root Motion Constant Force function in Unreal's Game Ability System
void AShooterCharacter::DashAbility()
{
	if (!bIsDashActive)
	{
			if(GetVelocity().Normalize()) // if character has velocity 
			{
				
				if (!GetCharacterMovement()->IsFalling())
				{
					bIsDashActive = true;
					if (DashSound)
					{
						UGameplayStatics::PlaySound2D(this, DashSound);
					}
					
					FVector LaunchVelocity = this->GetVelocity() * ForceMultiplier;
					LaunchCharacter(LaunchVelocity, true, false);
					GetMovementComponent()->StopMovementKeepPathing();
					GetWorldTimerManager().SetTimer(DashTimer, this, &AShooterCharacter::SetDashActiveToFalse, DashCooldown); //Ability cooldown
				}
			}
			
	}
}

// Set bIsDashActive for cooldown timer
void AShooterCharacter::SetDashActiveToFalse()
{
	bIsDashActive= false;
}

// Calculate Cross hair spread based on character's movement
void AShooterCharacter::CalculateCrossHairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange = FVector2D(0.f, 600.f);
	FVector2D VelocityMultiplierRange = FVector2D(0.f, 1.f);
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	if (GetCharacterMovement()->IsFalling()) // Is character in air?
	{
		// Spread crosshair slowly while in the air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else 
	{
		// Shrink the croshair rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}
	if (bAiming) // Is Character aiming?
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 20.f);
	}
	else // Character is not aiming
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 20.f);
	}
	// true 0.05 second after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuraiton);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	if (bSwitchToAuto)
	{
		bFireButtonPressed = true;
		StartFireTimer();
	}
	else
	{
		FireWeapon();
	}
	
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}

}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}

}
// Switch between full auto and singular shots
void AShooterCharacter::SwitchBetweenShootingModes()
{
	if (bSwitchToAuto)
	{
		bSwitchToAuto = false;
	}
	else
	{
		bSwitchToAuto = true;
	}

	UGameplayStatics::PlaySound2D(this, SwitchModeSound);

}


//Switches camera shoulder
void AShooterCharacter::SwitchCameraSides()
{	
	CameraYOffset *= -1.f;
	CameraBoom->SocketOffset.Y = CameraYOffset;

		
}

// Activate or deactive the slow motion
void AShooterCharacter::SlowMotionAbility()
{
	if(!bSlowMoActive)
	{
		bSlowMoActive = true;
		UGameplayStatics::PlaySound2D(this, SlowMoBeginSound);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);
	}
	else
	{
		bSlowMoActive = false;
		UGameplayStatics::PlaySound2D(this, SlowMoEndSound);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);

	}

}


// Spawn and control drone
void AShooterCharacter::DroneAbility()
{
	// If you possess the drone while character is in the air, character will be hang in air. This prevents that bug
	if (!GetCharacterMovement()->IsFalling()) 
	{
	
		FVector Location = FVector(GetActorLocation().X, GetActorLocation().Y + 50.f, GetActorLocation().Z + 200);
		FTransform DroneTransform = GetActorTransform();
		DroneTransform.SetLocation(Location); // Drone will spawn at top of the character

		// Spawn the drone
		MyDrone = GetWorld()->SpawnActorDeferred<ADrone>(Drone, DroneTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (MyDrone)
		{
			MyDrone->FinishSpawning(DroneTransform);
		}


		// If you possess the drone while running character will stuck in that running animation. This prevents that bug
		GetCharacterMovement()->StopMovementKeepPathing();

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->Possess(MyDrone); // Control the drone



		// Drone control duration can be changed via DroneTime
		GetWorldTimerManager().SetTimer(DroneTimerHandle, this, &AShooterCharacter::DroneToPlayer, DroneTime);
		

		// Needs a skill cooldown

		// Needs a max range 

	}
	
}


void AShooterCharacter::DroneToPlayer()
{
	
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->Possess(this); // posses player back
	
	MyDrone->Destroy(); // Destroy the drone
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraInterpZoom(DeltaTime);
	SetLookRates();
	CalculateCrossHairSpread(DeltaTime);

				
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AShooterCharacter::DashAbility);
	
	PlayerInputComponent->BindAction("SwitchBetweenWeaponModes", IE_Pressed, this, &AShooterCharacter::SwitchBetweenShootingModes);

	PlayerInputComponent->BindAction("SwitchCameraSides", IE_Pressed, this, &AShooterCharacter::SwitchCameraSides);

	PlayerInputComponent->BindAction("SlowMotion", IE_Pressed, this, &AShooterCharacter::SlowMotionAbility);

	PlayerInputComponent->BindAction("DroneAbility", IE_Pressed, this, &AShooterCharacter::DroneAbility);

	

}
