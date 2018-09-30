// Copywrite Jameson Tucker 2016

#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

/// look for attached Physics Handle
void UGrabber::FindPhysicsHandleComponent() {
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Physics handle missing for %s!"), *GetOwner()->GetName());
	}
}

void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent) {
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Input component missing for %s!"), *GetOwner()->GetName());
	}
}

void UGrabber::Grab() {
	/// LINE TRACE and see if reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent(); // gets the mesh in our case
	auto ActorHit = HitResult.GetActor();

	/// If we hit something then attach a physics handle
	if (ActorHit != nullptr) {
		if (!PhysicsHandle) { return; }
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation(),
			ComponentToGrab->GetOwner()->GetActorRotation()
		);
	}
}

void UGrabber::Release() 
{
	if (!PhysicsHandle) { return; }

	PhysicsHandle->ReleaseComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }
	// if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		// move the object that we're holding
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	/// Uncomment to test ray cast
	/*DrawDebugLine(
	GetWorld(),
	PlayerViewPointLocation,
	LineTraceEnd,
	FColor(255, 0, 0),
	false,
	0.f,
	0.f,
	10.f
	);*/

	/// Line-trace (Ray-cast) out to reach distance
	FHitResult HitResult;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	return HitResult;
}

FVector UGrabber::GetReachLineStart()
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation;
}

FVector UGrabber::GetReachLineEnd() 
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	FVector LineTraceDirection = PlayerViewPointRotation.Vector() * Reach;
	return PlayerViewPointLocation + LineTraceDirection;
}