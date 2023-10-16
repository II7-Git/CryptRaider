// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "Engine/World.h"
#include "Chaos/DebugDrawQueue.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UGrabber::Release()
{
	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		AActor *GrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		GrabbedActor->Tags.Remove("Grabbed");
		PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();
		PhysicsHandle->ReleaseComponent();
	}
}

void UGrabber::Grab()
{
	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();
	if (PhysicsHandle == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	bool HasHit = GetGrabbableInReach(HitResult);
	if (HasHit)
	{
		UPrimitiveComponent *HitComponent = HitResult.GetComponent();
		// 대상 컴포넌트의 RigidBody가 Sleep상태의 들어갔을 때 이를 깨워서 물리 시뮬레이트를 정상적으로 동작하게 한다.
		HitComponent->SetSimulatePhysics(true);

		HitComponent->WakeAllRigidBodies();
		AActor *HitActor = HitResult.GetActor();
		HitActor->Tags.Add("Grabbed");
		HitActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent,
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation());
	}
}

UPhysicsHandleComponent *UGrabber::GetPhysicsHandle() const
{
	UPhysicsHandleComponent *Result = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	if (Result == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Grabber requires a UPhysicsHandleComponent."));
	}

	return Result;
}

bool UGrabber::GetGrabbableInReach(FHitResult &OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	DrawDebugLine(GetWorld(), Start, End, FColor::Red);
	DrawDebugSphere(GetWorld(), End, 10, 10, FColor::Blue, false, 5);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);
	// FQuat: 회전값을 알려주는 객체 ,본문에 쓰인 FQuat::Identity는 회전값이 없다는 표현
	// SweepSingleByChannel():지정한 Collision Shape를 트레이스 시켜서 매개변수로 넣은 채널 소속의 첫번째 충돌체를 리턴하는 함수,충돌했다면 true리턴
	return GetWorld()->SweepSingleByChannel(
		OutHitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		Sphere);
}