#include "Systems/MovementSystem/Components/VelocityFeedback.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "Quaternion.h"


// Sets default values for this component's properties
UVelocityFeedback::UVelocityFeedback(): CameraComp(nullptr), PlayerController(nullptr), CameraShake(nullptr),
                                        SpeedLineEffect(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SpeedLineEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SpeedLines"));
}


// Called when the game starts
void UVelocityFeedback::BeginPlay()
{
	Super::BeginPlay();

	if (UActorComponent* Comp = GetOwner()->GetComponentByClass<UCameraComponent>())
	{
		CameraComp = Cast<UCameraComponent>(Comp);
		if (CameraComp) CameraComp->SetFieldOfView(MinFOV);
	}

	PlayerController = Cast<APlayerController>(GetOwner());
}


// Called every frame
void UVelocityFeedback::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float NormalizedVelocity = GetOwner()->GetVelocity().Size()/MaxVelocitySize; //TODO: i have to add a max speed in RollaBallPlayer
	
	if (CameraComp)
	{
		//FOV
		float LerpedFOV = FMath::Lerp(MinFOV, MaxFOV, NormalizedVelocity * FovMultiplier);
		if (LerpedFOV > MaxFOV) LerpedFOV = MaxFOV;
		/*if (LerpedFOV >= FovToEaseAt) //limit to max fov
			LerpedFOV = FMath::FInterpTo(FovToEaseAt, MaxFOV, DeltaTime, SpeedOfEasingIntoMaxFOV);*/
		CameraComp->SetFieldOfView(LerpedFOV);

		//SPEED LINES
		SpeedLineEffect->SetWorldLocation(CameraComp->GetComponentLocation());
		//Calculate Direction
		ThisFrameOwnerPosition = GetOwner()->GetActorLocation();
		MoveDirection = FVector
		(
			ThisFrameOwnerPosition.X-LastFrameOwnerPosition.X,
			ThisFrameOwnerPosition.Y-LastFrameOwnerPosition.Y,
			ThisFrameOwnerPosition.Z-LastFrameOwnerPosition.Z
		);
		MoveDirection = MoveDirection.GetSafeNormal();
		LastFrameOwnerPosition = GetOwner()->GetActorLocation();
		SpeedLineEffect->SetRelativeRotation(MoveDirection.ToOrientationQuat());
	}

	//CAMERA SHAKE
	//TODO: just get it working lol
	//TODO: has to be client, but how???
	//TODO: fix nesting
	if (PlayerController && CameraShake)
	{
		if (GetOwner()->GetVelocity().Size() > ShakeVelocityThreshold)
		{
			float LerpedScale = FMath::Lerp(MinShake, MaxShake, NormalizedVelocity * ShakeMultiplier);
			if (LerpedScale > MaxShake) LerpedScale = MaxShake;
			CameraShake->StartShake(PlayerController->PlayerCameraManager, LerpedScale, ECameraShakePlaySpace::CameraLocal, FRotator(0, 0, 0));
		}
		else CameraShake->StopShake();
	}
	
	//TODO: motion blur
	
}

