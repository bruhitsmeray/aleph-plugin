// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseChr.h"
#include "CSL_Window.h"
#include "Vitals.h"

#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

FString Sender;

// Sets default values
ABaseChr::ABaseChr()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Sender = this->GetName();
	
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanJump = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpeed;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	
	JumpMaxCount = 2;
	JumpMaxHoldTime = 0.1f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;
	Camera->SetWorldLocation(FVector(0,0,70));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Camera);
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 12.0f;

	InnerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("InnerLight"));
	InnerLight->SetupAttachment(SpringArm);
	InnerLight->AttenuationRadius = 2500.0f;
	InnerLight->InnerConeAngle = 16.0f;
	InnerLight->OuterConeAngle = 24.0f;
	InnerLight->SetVisibility(false);

	OuterLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("OuterLight"));
	OuterLight->SetupAttachment(SpringArm);
	OuterLight->Intensity = 2500.0f;
	OuterLight->AttenuationRadius = 2500.0f;
	OuterLight->InnerConeAngle = 32.0f;
	OuterLight->OuterConeAngle = 48.0f;
	OuterLight->SetVisibility(false);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	
	//	NOTE: In previous versions of this plugin, the cable was spawned alongside the other components, from now on
	//	the user has to create the cable component manually in Blueprints as it is easier to customize.	It's functionality
	//	however stays inside the code so that the user doesn't need to worry about that stuff.	
}

// Called when the game starts or when spawned
void ABaseChr::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseChr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	GrappleTick();
}

// Called to bind functionality to input
void ABaseChr::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(MoveForwardBindName, this, &ABaseChr::MoveForward);
	PlayerInputComponent->BindAxis(MoveSideBindName, this, &ABaseChr::MoveSide);
	PlayerInputComponent->BindAxis(VerticalLookBindName, this, &ABaseChr::VerticalLook);
	PlayerInputComponent->BindAxis(VerticalLookOnControllerBindName, this, &ABaseChr::VerticalLookOnController);
	PlayerInputComponent->BindAxis(HorizontalLookBindName, this, &ABaseChr::HorizontalLook);
	PlayerInputComponent->BindAxis(HorizontalLookOnControllerBindName, this, &ABaseChr::HorizontalLookOnController);

	PlayerInputComponent->BindAction(GrappleBindName, IE_Pressed, this, &ABaseChr::Grapple);
	PlayerInputComponent->BindAction(GrappleBindName, IE_Released, this, &ABaseChr::GrappleStop);
}

bool ABaseChr::IsMoving()
{
	if((InputComponent->GetAxisValue(MoveForwardBindName) > 0 || InputComponent->GetAxisValue(MoveForwardBindName) < 0)
		|| (InputComponent->GetAxisValue(MoveSideBindName) > 0 || InputComponent->GetAxisValue(MoveSideBindName) < 0))
	{
		return true;
	}
	return false;
}

void ABaseChr::MoveForward(float Value)
{
	if(Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
		IsWalkingV = true;
	} else
	{
		IsWalkingV = false;
	}
}

void ABaseChr::MoveSide(float Value)
{
	if(Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
		IsWalkingH = true;
	} else
	{
		IsWalkingH = false;
	}
}

void ABaseChr::VerticalLook(float Axis)
{
	AddControllerPitchInput(Axis * MouseSensitivity);
	GrabLocation();
}

void ABaseChr::VerticalLookOnController(float Axis)
{
	AddControllerPitchInput(Axis * SensitivityY * GetWorld()->GetDeltaSeconds());
	GrabLocation();
}

void ABaseChr::HorizontalLook(float Axis)
{
	AddControllerYawInput(Axis * MouseSensitivity);
	GrabLocation();
}

void ABaseChr::HorizontalLookOnController(float Axis)
{
	AddControllerYawInput(Axis * SensitivityZ * GetWorld()->GetDeltaSeconds());
	GrabLocation();
}

void ABaseChr::DevCam(int Mode)
{
	if(bCanUseCheats)
	{
		if(!Mode && !bDevWalk || Mode == 1)
		{
			bDevWalk = true;
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			SetActorEnableCollision(false);
			UCSL_Window::PrintToConsole(Sender, "Warning", "DevCam is now ON.");
		} else if(!Mode && bDevWalk || Mode == 0) {
			bDevWalk = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			SetActorEnableCollision(true);
			UCSL_Window::PrintToConsole(Sender, "Warning", "DevCam is now OFF.");
		}	
	} else {
		UCSL_Window::PrintToConsole(Sender, "Warning", "DevCam needs AllowCheats set to 1. Please set AllowCheats to 1 and try again!");
	}
}

void ABaseChr::AllowCheats(int Mode)
{
	if(Mode == 1)
	{
		bCanUseCheats = true;
		UCSL_Window::PrintToConsole(Sender, "Notice", FString::Printf(TEXT("AllowCheats changed to %i"), Mode));
	} else if(Mode == 0) {
		bCanUseCheats = false;
		UCSL_Window::PrintToConsole(Sender, "Notice", FString::Printf(TEXT("AllowCheats changed to %i"), Mode));
	}
}

void ABaseChr::GrabLocation()
{
	PhysicsHandle->SetTargetLocation(Camera->GetComponentLocation() + (Camera->GetForwardVector() * GrabDistance));
	if(bIsGrabbing && IsValid(HitComponent))
	{
		HitComponent->SetWorldRotation(GetControlRotation(), false, nullptr);
	}
}

bool ABaseChr::Trace(float Distance)
{
	if(UWorld* World = GetWorld())
	{
		APlayerCameraManager* ChrCamera = World->GetFirstPlayerController()->PlayerCameraManager;
		FVector CamWorldLocation = ChrCamera->GetCameraLocation();
		FVector CamForwardVec = ChrCamera->GetActorForwardVector() * Distance + CamWorldLocation;

		FCollisionQueryParams TraceParams;
		FHitResult HitResult;
		
		bool bDidTrace = GetWorld()->LineTraceSingleByChannel(HitResult, CamWorldLocation, CamForwardVec, ECC_Visibility, TraceParams);
		if(bDidTrace)
		{
			GrappleLocation = HitResult.Location;
			if(UKismetMathLibrary::Vector_Distance(GrappleLocation, GetActorLocation()) > 3072.0f)
			{
				bIsHookSuppressed = true;
				UCSL_Window::PrintToConsole(Sender, "Error", "The registered hit is too far away from the player.");
				return false;
			} else {
				bIsHookSuppressed = false;
				return HitResult.GetComponent()->IsSimulatingPhysics();
			}
		} else {
			bIsHookSuppressed = true;
			UCSL_Window::PrintToConsole(Sender, "Error", "No hit was registered.");
			return false;
		}
	}
	return false;
}

void ABaseChr::Grapple()
{
	Trace(GrappleDistance);
	if(bCanUseHook && !bIsHookSuppressed)
	{
		bIsGrappling = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetGrappleVisibility(true);
	}
}

void ABaseChr::GrappleTick()
{
	if(bCanUseHook && !bIsHookSuppressed && bIsGrappling)
	{
		GetCharacterMovement()->AddForce(UKismetMathLibrary::Normal(UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), GrappleLocation) + ((GetInputAxisValue(MoveSideBindName) * GetActorRightVector()) * 0.5f)) * 300000.0f);
		UCableComponent* Cable = FindComponentByClass<UCableComponent>();
		Cable->EndLocation = UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), GrappleLocation);
		if((UKismetMathLibrary::Vector_Distance(GrappleLocation, GetActorLocation()) <= 256.0f) || (UKismetMathLibrary::Vector_Distance(GrappleLocation, GetActorLocation()) >= 3072.0f))
		{
			GrappleStop();
		}
	}
}

void ABaseChr::GrappleStop()
{
	if(bCanUseHook)
	{
		bIsGrappling = false;
		bIsHookSuppressed = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		SetGrappleVisibility(false);	
	}
}

void ABaseChr::SetGrappleVisibility(bool Visibility)
{
	UCableComponent* Cable = FindComponentByClass<UCableComponent>();
	if(Cable->IsValidLowLevel())
	{
		Cable->SetVisibility(Visibility);
	} else
	{
		UCSL_Window::PrintToConsole(Sender, "Warning", "No cable component found! Please add a cable component to the Player Character blueprint in order to use the Grapple functions!");
	}
}

void ABaseChr::BeginSprint()
{
	if(IsMoving() && !bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * BaseWalkSpeedMultiplier;
		UCSL_Window::PrintToConsole(Sender, "Warning", FString::Printf(TEXT("MaxWalkSpeed is now at: %f (Sprinting/Dashing)."), GetCharacterMovement()->MaxWalkSpeed));
	}
}

void ABaseChr::StopSprint()
{
	if (GetCharacterMovement()->MaxWalkSpeed > BaseWalkSpeed)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		UCSL_Window::PrintToConsole(Sender, "Warning", FString::Printf(TEXT("MaxWalkSpeed is now at: %f (Walking)."), GetCharacterMovement()->MaxWalkSpeed));
	}
}

void ABaseChr::BeginDash(float AxisV, float AxisH, int Cost)
{
	if(bCanDash && IsMoving())
	{
		DashVertically(AxisV, Cost);
		DashHorizontally(AxisH, Cost);
	}
}

void ABaseChr::DashVertically(float AxisV, int Value)
{
	if(AxisV == 1.0f && IsWalkingV)
	{
		DashAxis = 5000.0f;
		if(!bIsCrouching || !bIsGrabbing || !bIsGrappling)
		{
			float LaunchX = GetActorForwardVector().X * DashAxis;
			float LaunchY = GetActorForwardVector().Y * DashAxis;
			float LaunchZ = GetActorForwardVector().Z;
			LaunchCharacter(UKismetMathLibrary::MakeVector(LaunchX, LaunchY, LaunchZ), false, false);
		}
	} else if(AxisV == -1.0f && IsWalkingV) {
		DashAxis = -5000.0f;
		if(!bIsCrouching || !bIsGrabbing || !bIsGrappling)
		{
			float LaunchX = GetActorForwardVector().X * DashAxis;
			float LaunchY = GetActorForwardVector().Y * DashAxis;
			float LaunchZ = GetActorForwardVector().Z;
			LaunchCharacter(UKismetMathLibrary::MakeVector(LaunchX, LaunchY, LaunchZ), false, false);
		}
	}
	UVitals::RemoveAuxPower(Value);
	UCSL_Window::PrintToConsole(Sender, "Warning", "DashVertically function called.");
}

void ABaseChr::DashHorizontally(float AxisH, int Value)
{
	if(AxisH == 1.0f && IsWalkingH)
	{
		DashAxis = 5000.0f;
		if(!bIsCrouching || !bIsGrabbing || !bIsGrappling)
		{
			float LaunchX = GetActorRightVector().X * DashAxis;
			float LaunchY = GetActorRightVector().Y * DashAxis;
			float LaunchZ = GetActorRightVector().Z;
			LaunchCharacter(UKismetMathLibrary::MakeVector(LaunchX, LaunchY, LaunchZ), false, false);
		}
	} else if(AxisH == -1.0f && IsWalkingH) {
		DashAxis = -5000.0f;
		if(!bIsCrouching || !bIsGrabbing || !bIsGrappling)
		{
			float LaunchX = GetActorRightVector().X * DashAxis;
			float LaunchY = GetActorRightVector().Y * DashAxis;
			float LaunchZ = GetActorRightVector().Z;
			LaunchCharacter(UKismetMathLibrary::MakeVector(LaunchX, LaunchY, LaunchZ), false, false);
		}
	}
	UVitals::RemoveAuxPower(Value);
	UCSL_Window::PrintToConsole(Sender, "Warning", "DashHorizontally function called");
}

