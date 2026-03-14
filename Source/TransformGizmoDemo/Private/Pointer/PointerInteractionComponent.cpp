#include "Pointer/PointerInteractionComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Pointer/PointerInteractableInterface.h"

UPointerInteractionComponent::UPointerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UPointerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoActivate)
	{
		EnableInteraction();
	}
	else
	{
		DisableInteraction();
	}
}

void UPointerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsActive())
	{
		return;
	}

	if (bIsDragging)
	{
		if (InteractionSettings.bEnableDragging)
		{
			DragPointer();
		}
		return;
	}

	UpdatePointerHover();
}

void UPointerInteractionComponent::SetupInteraction(const FPointerInteractionSettings& NewSettings, bool bActivate)
{
	DisableInteraction();
	InteractionSettings = NewSettings;
	if (bActivate)
	{
		EnableInteraction();
	}
}

void UPointerInteractionComponent::EnableInteraction()
{
	SetActive(true);
	SetComponentTickEnabled(true);
}

void UPointerInteractionComponent::DisableInteraction()
{
	SetActive(false);
	SetComponentTickEnabled(false);

	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		IPointerInteractableInterface::Execute_OnPointerUp(HitActor, this, HitComponent);
		IPointerInteractableInterface::Execute_OnPointerLeave(HitActor, this, HitComponent);
	}

	bIsDragging = false;
	HitActor = nullptr;
	HitComponent = nullptr;
	SavedHitResult = FHitResult();
}

void UPointerInteractionComponent::PressPointer()
{
	if (InteractionSettings.bHoverOnPress)
	{
		UpdatePointerHover();
	}

	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		bIsDragging = IPointerInteractableInterface::Execute_OnPointerDown(HitActor, this, HitComponent);
	}
}

void UPointerInteractionComponent::ReleasePointer()
{
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		IPointerInteractableInterface::Execute_OnPointerUp(HitActor, this, HitComponent);
	}
	bIsDragging = false;
}

void UPointerInteractionComponent::DragPointer()
{
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		IPointerInteractableInterface::Execute_OnPointerDrag(HitActor, this, HitComponent);
	}
}

void UPointerInteractionComponent::UpdatePointerHover()
{
	if (bIsHoverLocked)
	{
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
		{
			IPointerInteractableInterface::Execute_OnPointerMove(HitActor, this, HitComponent);
		}
		return;
	}

	AActor* TestActor = nullptr;
	UPrimitiveComponent* TestComponent = nullptr;

	switch (InteractionSettings.InteractionSource)
	{
	case EPointerInteractionSource::Custom:
		TestActor = SavedHitResult.GetActor();
		TestComponent = SavedHitResult.GetComponent();
		break;
	case EPointerInteractionSource::MouseCursor:
	case EPointerInteractionSource::ScreenCenter:
	case EPointerInteractionSource::World:
	default:
		SavedHitResult = RaycastTest();
		TestActor = SavedHitResult.GetActor();
		TestComponent = SavedHitResult.GetComponent();
		break;
	}

	if (TestActor == HitActor && TestComponent == HitComponent)
	{
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
		{
			IPointerInteractableInterface::Execute_OnPointerMove(HitActor, this, HitComponent);
		}
		return;
	}

	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		IPointerInteractableInterface::Execute_OnPointerLeave(HitActor, this, HitComponent);
	}

	HitActor = TestActor;
	HitComponent = TestComponent;

	if (HitActor && HitActor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		IPointerInteractableInterface::Execute_OnPointerEnter(HitActor, this, HitComponent);
	}
}

FHitResult UPointerInteractionComponent::RaycastTest() const
{
	if (!GetWorld())
	{
		return FHitResult();
	}

	FVector RayStart;
	FVector RayEnd;
	GetPointerRay(RayStart, RayEnd);

	FHitResult OutHit;
	const FCollisionObjectQueryParams ObjectParams = MakeObjectQueryParams();
	const FCollisionQueryParams QueryParams = MakeQueryParams();

	if (InteractionSettings.bUseMultiRayHit)
	{
		TArray<FHitResult> Hits;
		const bool bHit = InteractionSettings.RayWidth > 0.0f
			? GetWorld()->SweepMultiByObjectType(Hits, RayStart, RayEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(InteractionSettings.RayWidth * 0.5f), QueryParams)
			: GetWorld()->LineTraceMultiByObjectType(Hits, RayStart, RayEnd, ObjectParams, QueryParams);

		if (bHit && SelectBestHit(Hits, OutHit))
		{
			return OutHit;
		}
		return FHitResult();
	}

	if (InteractionSettings.RayWidth > 0.0f)
	{
		if (GetWorld()->SweepSingleByObjectType(OutHit, RayStart, RayEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(InteractionSettings.RayWidth * 0.5f), QueryParams)
			&& CanInteractWith(OutHit.GetActor(), OutHit.GetComponent()))
		{
			return OutHit;
		}
	}
	else
	{
		if (GetWorld()->LineTraceSingleByObjectType(OutHit, RayStart, RayEnd, ObjectParams, QueryParams)
			&& CanInteractWith(OutHit.GetActor(), OutHit.GetComponent()))
		{
			return OutHit;
		}
	}

	return FHitResult();
}

void UPointerInteractionComponent::SetCustomHitResult(const FHitResult& InHitResult)
{
	SavedHitResult = InHitResult;
}

FHitResult UPointerInteractionComponent::GetLastHitResult() const
{
	return SavedHitResult;
}

void UPointerInteractionComponent::GetHoveredObject(AActor*& OutActor, UPrimitiveComponent*& OutComponent) const
{
	OutActor = HitActor;
	OutComponent = HitComponent;
}

void UPointerInteractionComponent::SetLockHover(bool bInLock)
{
	bIsHoverLocked = bInLock;
}

void UPointerInteractionComponent::GetPointerRay(FVector& OutRayStart, FVector& OutRayEnd) const
{
	OutRayStart = GetComponentLocation();
	OutRayEnd = OutRayStart + GetForwardVector() * InteractionSettings.InteractionDistance;

	APlayerController* PC = ResolvePlayerController();
	if (!PC)
	{
		return;
	}

	FVector WorldLoc;
	FVector WorldDir;

	switch (InteractionSettings.InteractionSource)
	{
	case EPointerInteractionSource::MouseCursor:
		if (PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir))
		{
			OutRayStart = WorldLoc;
			OutRayEnd = WorldLoc + WorldDir * InteractionSettings.InteractionDistance;
		}
		break;
	case EPointerInteractionSource::ScreenCenter:
		{
			int32 SizeX = 0;
			int32 SizeY = 0;
			PC->GetViewportSize(SizeX, SizeY);
			if (PC->DeprojectScreenPositionToWorld(SizeX * 0.5f, SizeY * 0.5f, WorldLoc, WorldDir))
			{
				OutRayStart = WorldLoc;
				OutRayEnd = WorldLoc + WorldDir * InteractionSettings.InteractionDistance;
			}
		}
		break;
	case EPointerInteractionSource::World:
	case EPointerInteractionSource::Custom:
	default:
		break;
	}
}

bool UPointerInteractionComponent::CanInteractWith(AActor* Actor, UPrimitiveComponent* Component) const
{
	if (!Actor || !Component)
	{
		return false;
	}

	if (!Actor->GetClass()->ImplementsInterface(UPointerInteractableInterface::StaticClass()))
	{
		return false;
	}

	if (IPointerInteractableInterface::Execute_IgnoreHitTest(Actor, const_cast<UPointerInteractionComponent*>(this), Component))
	{
		return false;
	}

	if (!InteractionSettings.OnlyActors.IsEmpty() && !InteractionSettings.OnlyActors.Contains(Actor))
	{
		return false;
	}

	return true;
}

APlayerController* UPointerInteractionComponent::ResolvePlayerController() const
{
	if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		return PawnOwner->GetController<APlayerController>();
	}
	return GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

FCollisionObjectQueryParams UPointerInteractionComponent::MakeObjectQueryParams() const
{
	FCollisionObjectQueryParams Params;
	if (InteractionSettings.InteractableTypes.IsEmpty())
	{
		Params.AddObjectTypesToQuery(ECC_WorldDynamic);
		return Params;
	}

	for (const TEnumAsByte<EObjectTypeQuery> ObjectType : InteractionSettings.InteractableTypes)
	{
		const ECollisionChannel Channel = UEngineTypes::ConvertToCollisionChannel(ObjectType);
		Params.AddObjectTypesToQuery(Channel);
	}
	return Params;
}

FCollisionQueryParams UPointerInteractionComponent::MakeQueryParams() const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PointerInteraction), InteractionSettings.bHitComplex);
	if (const AActor* OwnerActor = GetOwner())
	{
		Params.AddIgnoredActor(OwnerActor);
	}
	for (AActor* Ignored : InteractionSettings.IgnoredActors)
	{
		if (Ignored)
		{
			Params.AddIgnoredActor(Ignored);
		}
	}
	return Params;
}

bool UPointerInteractionComponent::SelectBestHit(const TArray<FHitResult>& Hits, FHitResult& OutHit) const
{
	int32 BestPriority = TNumericLimits<int32>::Lowest();
	float BestDistance = TNumericLimits<float>::Max();
	bool bFound = false;

	for (const FHitResult& Hit : Hits)
	{
		AActor* Actor = Hit.GetActor();
		UPrimitiveComponent* Component = Hit.GetComponent();
		if (!CanInteractWith(Actor, Component))
		{
			continue;
		}

		const int32 Priority = IPointerInteractableInterface::Execute_GetHitPriority(Actor, const_cast<UPointerInteractionComponent*>(this), Component);
		if (!bFound || Priority > BestPriority || (Priority == BestPriority && Hit.Distance < BestDistance))
		{
			BestPriority = Priority;
			BestDistance = Hit.Distance;
			OutHit = Hit;
			bFound = true;
		}
	}

	return bFound;
}
