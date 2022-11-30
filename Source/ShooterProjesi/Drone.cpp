

#include "Drone.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterCharacter.h"
#include "Engine/StaticMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


// Sets default values
ADrone::ADrone()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	SetRootComponent(DroneMesh);
	DroneMesh->SetSimulatePhysics(true);
	DroneMesh->SetLinearDamping(1.f);
	DroneMesh->SetAngularDamping(1.f);
	DroneMesh->SetEnableGravity(false);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(DroneMesh);
	SpringArm->SocketOffset= FVector(0.f, 0.f, 180.f);
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->TargetArmLength = 250.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	MoveForwardValue=0.f;
	MoveRightValue=0.f;
	MoveUpValue=0.f;

	MovementSpeed = 200.f;
	CameraSpeed = 7.f;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bIsBoostReady = true;



}

// Called when the game starts or when spawned
void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
}


void ADrone::MoveForward(float Value)
{
	MoveForwardValue = Value;
	DroneMovement();
}

void ADrone::MoveRight(float Value)
{
	MoveRightValue = Value;
	DroneMovement();
}

void ADrone::MoveUp(float Value)
{
	MoveUpValue = Value;
	DroneMovement();
}

// Update actor's velocity based on MoveForwardValue, MoveRightValue, MoveUpValue
void ADrone::DroneMovement()
{
	FVector MadeVector = UKismetMathLibrary::MakeVector(MoveForwardValue, MoveRightValue, MoveUpValue);
	FRotator MadeRotator = FRotator(0.f, Camera->GetComponentRotation().Yaw, 0.f);

	MadeVector = MadeRotator.RotateVector(MadeVector);

	MadeVector = MadeVector * MovementSpeed *  UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

	MadeVector = MadeVector + GetVelocity();

	DroneMesh->SetPhysicsLinearVelocity(MadeVector);

	
	
}

// Rotate spring arm component
void ADrone::UpdateSpringArm()
{
	FRotator SpringRotation = SpringArm->GetComponentRotation();
	float SpringArmYaw = SpringRotation.Yaw;
	float SpringArmPitch = SpringRotation.Pitch;

	float RotatorPitch = (CameraSpeed * (-1.f) * LookUpValue) + SpringArmPitch;
	RotatorPitch =	UKismetMathLibrary::ClampAngle(RotatorPitch,-45.f, 85.f);
	

	float RotatorYaw = (TurnValue * CameraSpeed) + SpringArmYaw;

	FRotator SpringArmTargetRotation  = UKismetMathLibrary::MakeRotator(0.f, RotatorPitch, RotatorYaw);
	

	SpringArm->SetWorldRotation(UKismetMathLibrary::RInterpTo(SpringRotation, SpringArmTargetRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 8.f));



	

}

// Rotate camera to drone
void ADrone::RotateCameraFocus()
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Camera->GetComponentLocation(), GetActorLocation());
	Camera->SetWorldRotation(UKismetMathLibrary::RInterpTo(Camera->GetComponentRotation(), LookAtRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 2.f)) ;


	FRotator ActorNewRotation = UKismetMathLibrary::MakeRotator((MoveRightValue * (20.f)), (MoveForwardValue * (-7.f)), LookAtRotation.Yaw);
	ActorNewRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), ActorNewRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 4.f);
	
	SetActorRotation(ActorNewRotation);
}

// Update camera fiel of view based on drone's velocity
void ADrone::UpdateCameraFOV()
{
	float VelocityLen = GetVelocity().Length();

	VelocityLen = UKismetMathLibrary::MapRangeUnclamped(VelocityLen, 300.f, 1'000.f, 75.f, 110.f);
	VelocityLen = UKismetMathLibrary::Clamp(VelocityLen, 75.f, 110.f);

	float FOV =	UKismetMathLibrary::FInterpTo(Camera->FieldOfView, VelocityLen, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 8.f);

	Camera->SetFieldOfView(FOV);
	

}

// Small dash based on drone's velocity 
void ADrone::DroneDash()
{
	if (bIsBoostReady)
	{
		bIsBoostReady = false;
		
		FVector BoostVector = GetVelocity() * 2.f;
		DroneMesh->SetPhysicsLinearVelocity(BoostVector);
		GetWorldTimerManager().SetTimer(BoostTimer, this, &ADrone::SetBoostState, 5.f);
	
	}



}

void ADrone::SetBoostState()
{
	if (!bIsBoostReady)
	{
		bIsBoostReady = true;
	}
}

void ADrone::Fire()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const FTransform SocketTransform = DroneMesh->GetSocketTransform("DroneBarrel");

	FVector BeamEnd;
	bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

	if (bBeamEnd)
	{
		// Spawn impact particles after updating BeamEndPoint
		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEnd);
		}


	}
}

bool ADrone::GetBeamEndLocation(const FVector & DroneSocketLocation, FVector & EndLocation)
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

		// Set end location to line trace end point
		EndLocation = End;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		// Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

		if (ScreenTraceHit.bBlockingHit) // did first trace hit?
		{
			//End location is now trace hit location
			EndLocation = ScreenTraceHit.Location;

			// This is for the testing, damage system is not complete
			UGameplayStatics::ApplyDamage(ScreenTraceHit.GetActor(), 1.f, nullptr, nullptr, nullptr);

			// Second trace from drone barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart = DroneSocketLocation;
			const FVector WeaponTraceEnd = EndLocation;
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);

			if (WeaponTraceHit.bBlockingHit)
			{
				EndLocation = WeaponTraceHit.Location;

			}
			return true;
		}
	}
	return false;

}


void ADrone::Turn(float Value)
{
	TurnValue = Value;
}

void ADrone::LookUp(float Value)
{
	LookUpValue = Value;
}


void ADrone::LookUpAtRate(float Rate)
{
	TurnValue = Rate;
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); 

}


void ADrone::TurnAtRate(float Rate)
{
	LookUpValue = Rate;
	//AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

// Called every frame
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpringArm();
	RotateCameraFocus();
	UpdateCameraFOV();

}

// Called to bind functionality to input
void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADrone::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADrone::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADrone::MoveUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &ADrone::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADrone::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &ADrone::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ADrone::LookUp);


	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ADrone::DroneDash);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ADrone::Fire);

}
