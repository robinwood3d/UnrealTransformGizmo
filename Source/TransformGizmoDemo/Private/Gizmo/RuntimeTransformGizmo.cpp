#include "Gizmo/RuntimeTransformGizmo.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Pointer/PointerInteractionComponent.h"

ARuntimeTransformGizmo::ARuntimeTransformGizmo()
{
	PrimaryActorTick.bCanEverTick = true;

	TransformGizmoGroup = CreateDefaultSubobject<USceneComponent>(TEXT("TransformGizmoGroup"));
	SetRootComponent(TransformGizmoGroup);

	TranslationGizmoGroup = CreateDefaultSubobject<USceneComponent>(TEXT("TranslationGizmoGroup"));
	TranslationGizmoGroup->SetupAttachment(TransformGizmoGroup);

	RotationGizmoGroup = CreateDefaultSubobject<USceneComponent>(TEXT("RotationGizmoGroup"));
	RotationGizmoGroup->SetupAttachment(TransformGizmoGroup);

	ScaleGizmoGroup = CreateDefaultSubobject<USceneComponent>(TEXT("ScaleGizmoGroup"));
	ScaleGizmoGroup->SetupAttachment(TransformGizmoGroup);

	TranslationHandle_X = CreateHandle(TEXT("TranslationHandle_X"), TranslationGizmoGroup);
	TranslationHandle_Y = CreateHandle(TEXT("TranslationHandle_Y"), TranslationGizmoGroup);
	TranslationHandle_Z = CreateHandle(TEXT("TranslationHandle_Z"), TranslationGizmoGroup);
	PlaneTranslationHandle_XY = CreateHandle(TEXT("PlaneTranslationHandle_XY"), TranslationGizmoGroup);
	PlaneTranslationHandle_XZ = CreateHandle(TEXT("PlaneTranslationHandle_XZ"), TranslationGizmoGroup);
	PlaneTranslationHandle_YZ = CreateHandle(TEXT("PlaneTranslationHandle_YZ"), TranslationGizmoGroup);
	ScreenTranslationHandle = CreateHandle(TEXT("ScreenTranslationHandle"), TranslationGizmoGroup);

	RotationHandle_X = CreateHandle(TEXT("RotationHandle_X"), RotationGizmoGroup);
	RotationHandle_Y = CreateHandle(TEXT("RotationHandle_Y"), RotationGizmoGroup);
	RotationHandle_Z = CreateHandle(TEXT("RotationHandle_Z"), RotationGizmoGroup);
	TrackballRotationHandle = CreateHandle(TEXT("TrackballRotationHandle"), RotationGizmoGroup);
	ScreenRotationHandle = CreateHandle(TEXT("ScreenRotationHandle"), RotationGizmoGroup);

	ScaleHandle_X = CreateHandle(TEXT("ScaleHandle_X"), ScaleGizmoGroup);
	ScaleHandle_Y = CreateHandle(TEXT("ScaleHandle_Y"), ScaleGizmoGroup);
	ScaleHandle_Z = CreateHandle(TEXT("ScaleHandle_Z"), ScaleGizmoGroup);
	PlaneScaleHandle_XY = CreateHandle(TEXT("PlaneScaleHandle_XY"), ScaleGizmoGroup);
	PlaneScaleHandle_XZ = CreateHandle(TEXT("PlaneScaleHandle_XZ"), ScaleGizmoGroup);
	PlaneScaleHandle_YZ = CreateHandle(TEXT("PlaneScaleHandle_YZ"), ScaleGizmoGroup);
	UniformScaleHandle = CreateHandle(TEXT("UniformScaleHandle"), ScaleGizmoGroup);

	SetupDefaultHandleTransforms();
}

void ARuntimeTransformGizmo::BeginPlay()
{
	Super::BeginPlay();
	InitializeGizmoHandleMap();
	if (bDumpHandleTransformsOnBeginPlay)
	{
		DumpHandleRelativeTransforms();
	}
	DeactivateGizmo();
}

void ARuntimeTransformGizmo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleasePointerLock();
	Super::EndPlay(EndPlayReason);
}

void ARuntimeTransformGizmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsDragging)
	{
		UpdateGizmoScale();
	}
}

void ARuntimeTransformGizmo::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (bUseDefaultHandleLayout)
	{
		SetupDefaultHandleTransforms();
	}
	InitializeGizmoHandleMap();
}

void ARuntimeTransformGizmo::ActivateGizmo(const FTransform& InitialTransform)
{
	bIsActivated = true;
	SetActorTickEnabled(true);

	WorldLocation = InitialTransform.GetLocation();
	WorldRotation = InitialTransform.Rotator();
	WorldScale = InitialTransform.GetScale3D();

	SetAlignSpace(AlignSpace);
	UpdateGizmoScale();
	SetTransformMode(ActiveTransformMode, bCombineTranslateRotate);
}

void ARuntimeTransformGizmo::DeactivateGizmo()
{
	ReleasePointerLock();
	bIsActivated = false;
	bIsDragging = false;
	ActiveDragHandle = nullptr;
	SetActorTickEnabled(false);
	SetActiveGizmoGroup(false, false, false);
}

void ARuntimeTransformGizmo::SetTransformMode(ETransformMode InTransformMode, bool bInCombineTranslateRotate)
{
	ActiveTransformMode = InTransformMode;
	bCombineTranslateRotate = bInCombineTranslateRotate;

	if (!bIsActivated)
	{
		return;
	}

	if (bCombineTranslateRotate)
	{
		SetActiveGizmoGroup(true, true, false);
		return;
	}

	switch (ActiveTransformMode)
	{
	case ETransformMode::Translation:
		SetActiveGizmoGroup(true, false, false);
		break;
	case ETransformMode::Rotation:
		SetActiveGizmoGroup(false, true, false);
		break;
	case ETransformMode::Scale:
		SetActiveGizmoGroup(false, false, true);
		break;
	}
}

void ARuntimeTransformGizmo::SetAlignSpace(EAlignSpace InAlignSpace)
{
	AlignSpace = InAlignSpace;
	if (bIsActivated)
	{
		ApplyGizmoTransform(WorldLocation, WorldRotation, WorldScale);
	}
}

void ARuntimeTransformGizmo::SetTransformValues(const FVector& InLocation, const FRotator& InRotation, const FVector& InScale)
{
	WorldLocation = InLocation;
	WorldRotation = InRotation;
	WorldScale = InScale;
	ApplyGizmoTransform(WorldLocation, WorldRotation, WorldScale);
}

void ARuntimeTransformGizmo::SetActiveGizmoGroup(bool bTranslation, bool bRotation, bool bScale)
{
	auto SetGroupVisible = [](USceneComponent* Group, bool bVisible)
	{
		if (!Group)
		{
			return;
		}

		TArray<USceneComponent*> Children;
		Group->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Child))
			{
				Prim->SetVisibility(bVisible);
				Prim->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			}
		}
	};

	SetGroupVisible(TranslationGizmoGroup, bTranslation);
	SetGroupVisible(RotationGizmoGroup, bRotation);
	SetGroupVisible(ScaleGizmoGroup, bScale);
}

void ARuntimeTransformGizmo::InitializeGizmoHandleMap()
{
	GizmoHandleMap.Empty();

	RegisterHandle(TranslationHandle_X, {ETransformMode::Translation, EConstraintAxis::X, BaseInteractionPriority + 3});
	RegisterHandle(TranslationHandle_Y, {ETransformMode::Translation, EConstraintAxis::Y, BaseInteractionPriority + 3});
	RegisterHandle(TranslationHandle_Z, {ETransformMode::Translation, EConstraintAxis::Z, BaseInteractionPriority + 3});
	RegisterHandle(PlaneTranslationHandle_XY, {ETransformMode::Translation, EConstraintAxis::XY, BaseInteractionPriority + 2});
	RegisterHandle(PlaneTranslationHandle_XZ, {ETransformMode::Translation, EConstraintAxis::XZ, BaseInteractionPriority + 2});
	RegisterHandle(PlaneTranslationHandle_YZ, {ETransformMode::Translation, EConstraintAxis::YZ, BaseInteractionPriority + 2});
	RegisterHandle(ScreenTranslationHandle, {ETransformMode::Translation, EConstraintAxis::Screen, BaseInteractionPriority + 4});

	RegisterHandle(RotationHandle_X, {ETransformMode::Rotation, EConstraintAxis::X, BaseInteractionPriority + 1});
	RegisterHandle(RotationHandle_Y, {ETransformMode::Rotation, EConstraintAxis::Y, BaseInteractionPriority + 1});
	RegisterHandle(RotationHandle_Z, {ETransformMode::Rotation, EConstraintAxis::Z, BaseInteractionPriority + 1});
	RegisterHandle(TrackballRotationHandle, {ETransformMode::Rotation, EConstraintAxis::XYZ, BaseInteractionPriority + 1});
	RegisterHandle(ScreenRotationHandle, {ETransformMode::Rotation, EConstraintAxis::Screen, BaseInteractionPriority});

	RegisterHandle(ScaleHandle_X, {ETransformMode::Scale, EConstraintAxis::X, BaseInteractionPriority + 1});
	RegisterHandle(ScaleHandle_Y, {ETransformMode::Scale, EConstraintAxis::Y, BaseInteractionPriority + 1});
	RegisterHandle(ScaleHandle_Z, {ETransformMode::Scale, EConstraintAxis::Z, BaseInteractionPriority + 1});
	RegisterHandle(PlaneScaleHandle_XY, {ETransformMode::Scale, EConstraintAxis::XY, BaseInteractionPriority});
	RegisterHandle(PlaneScaleHandle_XZ, {ETransformMode::Scale, EConstraintAxis::XZ, BaseInteractionPriority});
	RegisterHandle(PlaneScaleHandle_YZ, {ETransformMode::Scale, EConstraintAxis::YZ, BaseInteractionPriority});
	RegisterHandle(UniformScaleHandle, {ETransformMode::Scale, EConstraintAxis::XYZ, BaseInteractionPriority + 2});
}

void ARuntimeTransformGizmo::UpdateGizmoScale()
{
	if (!bIsActivated)
	{
		return;
	}

	const float ViewportScale = bScaledByDPI ? FMath::Max(0.01f, UWidgetLayoutLibrary::GetViewportScale(this)) : 1.0f;
	const float ScreenSize = FMath::Max(0.01f, GizmoScreenSize / ViewportScale);

	if (IsOrthographicView())
	{
		GizmoWorldScale = 0.001f * GetOrthoWidth() * ScreenSize;
	}
	else
	{
		const float Distance = FVector::DotProduct(GetViewDirection(), WorldLocation - GetViewLocation());
		GizmoWorldScale = Distance * 0.001f * ScreenSize * FMath::Tan(FMath::DegreesToRadians(GetFieldOfView() * 0.5f));
	}

	GizmoWorldScale = FMath::Clamp(GizmoWorldScale, 0.01f, 10.0f);
	SetActorScale3D(FVector(GizmoWorldScale));
}

FVector ARuntimeTransformGizmo::GetSnappedLocation(const FVector& LocationDelta) const
{
	if (!bEnableLocationSnap)
	{
		return LocationDelta;
	}

	return FVector(
		FMath::GridSnap(LocationDelta.X, LocationGridSize),
		FMath::GridSnap(LocationDelta.Y, LocationGridSize),
		FMath::GridSnap(LocationDelta.Z, LocationGridSize)
	);
}

float ARuntimeTransformGizmo::GetSnappedAngle(float Angle) const
{
	if (!bEnableRotationSnap)
	{
		return Angle;
	}
	return FMath::GridSnap(Angle, RotationGridSize);
}

float ARuntimeTransformGizmo::GetSnappedScale(float ScaleDelta) const
{
	if (!bEnableScaleSnap)
	{
		return ScaleDelta;
	}
	return FMath::GridSnap(ScaleDelta, ScaleGridSize);
}

int32 ARuntimeTransformGizmo::GetHitPriority_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const
{
	if (const FGizmoHandleInfo* Info = GizmoHandleMap.Find(Component))
	{
		return Info->HitPriority;
	}
	return TNumericLimits<int32>::Lowest();
}

bool ARuntimeTransformGizmo::IgnoreHitTest_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const
{
	return !bIsActivated || !GizmoHandleMap.Contains(Component);
}

bool ARuntimeTransformGizmo::OnPointerDown_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
	if (!Pointer || !Component || !GizmoHandleMap.Contains(Component) || !bIsActivated)
	{
		return false;
	}

	ActivePointer = Pointer;
	Pointer->SetLockHover(true);

	FVector RayStart;
	FVector RayEnd;
	Pointer->GetPointerRay(RayStart, RayEnd);

	if (!BuildDragContext(Component, RayStart, RayEnd))
	{
		ReleasePointerLock(Pointer);
		return false;
	}

	bIsDragging = true;
	ActiveDragHandle = Component;
	OnBeginTransform.Broadcast(FTransform(WorldRotation, WorldLocation, WorldScale));
	return true;
}

void ARuntimeTransformGizmo::OnPointerDrag_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
	if (!bIsDragging || !Pointer || Component != ActiveDragHandle)
	{
		return;
	}

	FVector RayStart;
	FVector RayEnd;
	Pointer->GetPointerRay(RayStart, RayEnd);

	FVector CurrentDragLocation;
	if (!IntersectRayConstraintPlane(RayStart, RayEnd, CurrentDragLocation))
	{
		return;
	}

	const FVector Delta = CurrentDragLocation - StartDragLocation;

	switch (ActiveDragHandleInfo.TransformMode)
	{
	case ETransformMode::Translation:
		{
			FVector DeltaToApply = Delta;
			if (ActiveDragHandleInfo.ConstraintAxis == EConstraintAxis::X || ActiveDragHandleInfo.ConstraintAxis == EConstraintAxis::Y || ActiveDragHandleInfo.ConstraintAxis == EConstraintAxis::Z)
			{
				const FVector Axis = GetAxisVector(ActiveDragHandleInfo.ConstraintAxis).GetSafeNormal();
				DeltaToApply = FVector::DotProduct(DeltaToApply, Axis) * Axis;
			}
			WorldLocation = WorldLocationOnDragStart + GetSnappedLocation(DeltaToApply);
		}
		break;
	case ETransformMode::Rotation:
		{
			FVector RotationAxis = GetAxisVector(ActiveDragHandleInfo.ConstraintAxis).GetSafeNormal();
			if (ActiveDragHandleInfo.ConstraintAxis == EConstraintAxis::Screen)
			{
				const FVector A = (LastDragLocation - WorldLocation).GetSafeNormal();
				const FVector B = (CurrentDragLocation - WorldLocation).GetSafeNormal();
				const float UnsignedAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(A, B), -1.0f, 1.0f)));
				const float Sign = FMath::Sign(FVector::DotProduct(FVector::CrossProduct(A, B).GetSafeNormal(), GetDirectionToView()));
				DraggedAngle += UnsignedAngle * Sign;
				RotationAngle = GetSnappedAngle(DraggedAngle);
				RotationAxis = GetDirectionToView();
			}
			else
			{
				const FVector CameraFactor = FVector::CrossProduct(RotationAxis, GetDirectionToView());
				const float Angle = FVector::DotProduct(CameraFactor, Delta) / FMath::Max(0.001f, GizmoWorldScale) * RotateFactor;
				RotationAngle = GetSnappedAngle(Angle);
			}

			WorldRotation = (FQuat(RotationAxis, FMath::DegreesToRadians(RotationAngle)) * WorldRotationOnDragStart.Quaternion()).Rotator();
		}
		break;
	case ETransformMode::Scale:
		{
			FVector AxisMask = FVector::OneVector;
			switch (ActiveDragHandleInfo.ConstraintAxis)
			{
			case EConstraintAxis::X: AxisMask = FVector(1, 0, 0); break;
			case EConstraintAxis::Y: AxisMask = FVector(0, 1, 0); break;
			case EConstraintAxis::Z: AxisMask = FVector(0, 0, 1); break;
			case EConstraintAxis::XY: AxisMask = FVector(1, 1, 0); break;
			case EConstraintAxis::XZ: AxisMask = FVector(1, 0, 1); break;
			case EConstraintAxis::YZ: AxisMask = FVector(0, 1, 1); break;
			case EConstraintAxis::XYZ:
			case EConstraintAxis::Screen:
			default:
				AxisMask = FVector(1, 1, 1);
				break;
			}

			const FVector AxisWorld = (AlignRotator.RotateVector(AxisMask)).GetSafeNormal();
			const float Magnitude = FVector::DotProduct(Delta, AxisWorld) * ScaleFactor * 0.02f;
			const float Snapped = GetSnappedScale(Magnitude / FMath::Max(0.001f, (WorldScaleOnDragStart * AxisMask).Size()));
			WorldScale = WorldScaleOnDragStart + (WorldScaleOnDragStart * AxisMask) * Snapped;
			WorldScale.X = FMath::Max(0.001f, WorldScale.X);
			WorldScale.Y = FMath::Max(0.001f, WorldScale.Y);
			WorldScale.Z = FMath::Max(0.001f, WorldScale.Z);
		}
		break;
	}

	LastDragLocation = CurrentDragLocation;
	UpdateGizmoScale();
	ApplyGizmoTransform(WorldLocation, WorldRotation, WorldScale);
	OnUpdateTransform.Broadcast(FTransform(WorldRotation, WorldLocation, WorldScale));
}

void ARuntimeTransformGizmo::OnPointerEnter_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
}

void ARuntimeTransformGizmo::OnPointerLeave_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
}

void ARuntimeTransformGizmo::OnPointerMove_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
}

void ARuntimeTransformGizmo::OnPointerUp_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component)
{
	if (!bIsDragging)
	{
		ReleasePointerLock(Pointer);
		return;
	}

	bIsDragging = false;
	ActiveDragHandle = nullptr;
	ReleasePointerLock(Pointer);
	OnFinishTransform.Broadcast(FTransform(WorldRotation, WorldLocation, WorldScale));
}

UStaticMeshComponent* ARuntimeTransformGizmo::CreateHandle(const TCHAR* Name, USceneComponent* Parent)
{
	UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Mesh->SetupAttachment(Parent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	Mesh->SetGenerateOverlapEvents(false);
	return Mesh;
}

void ARuntimeTransformGizmo::RegisterHandle(UPrimitiveComponent* Component, const FGizmoHandleInfo& Info)
{
	if (Component)
	{
		GizmoHandleMap.Add(Component, Info);
	}
}

bool ARuntimeTransformGizmo::BuildDragContext(UPrimitiveComponent* Handle, const FVector& RayStart, const FVector& RayEnd)
{
	const FGizmoHandleInfo* Info = GizmoHandleMap.Find(Handle);
	if (!Info)
	{
		return false;
	}

	ActiveDragHandleInfo = *Info;
	WorldLocationOnDragStart = WorldLocation;
	WorldRotationOnDragStart = WorldRotation;
	WorldScaleOnDragStart = WorldScale;
	AlignRotator = (AlignSpace == EAlignSpace::Local || Info->TransformMode == ETransformMode::Scale) ? WorldRotationOnDragStart : FRotator::ZeroRotator;
	RotationAngle = 0.0f;
	DraggedAngle = 0.0f;

	FVector PlaneNormal;
	const FVector AxisVector = GetAxisVector(Info->ConstraintAxis, true).GetSafeNormal();

	if (Info->TransformMode == ETransformMode::Rotation)
	{
		PlaneNormal = GetDirectionToView();
	}
	else
	{
		switch (Info->ConstraintAxis)
		{
		case EConstraintAxis::X:
		case EConstraintAxis::Y:
		case EConstraintAxis::Z:
		case EConstraintAxis::XYZ:
			PlaneNormal = FVector::CrossProduct(AxisVector, FVector::CrossProduct(AxisVector, GetDirectionToView())).GetSafeNormal();
			if (PlaneNormal.IsNearlyZero())
			{
				PlaneNormal = GetDirectionToView();
			}
			break;
		case EConstraintAxis::XY:
		case EConstraintAxis::XZ:
		case EConstraintAxis::YZ:
		case EConstraintAxis::Screen:
		default:
			PlaneNormal = AxisVector.IsNearlyZero() ? GetDirectionToView() : AxisVector;
			break;
		}
	}

	DragConstraintPlane = FPlane(WorldLocation, PlaneNormal.GetSafeNormal());
	if (!IntersectRayConstraintPlane(RayStart, RayEnd, StartDragLocation))
	{
		return false;
	}
	LastDragLocation = StartDragLocation;
	return true;
}

void ARuntimeTransformGizmo::ApplyGizmoTransform(const FVector& InLocation, const FRotator& InRotation, const FVector& InScale)
{
	if (AlignSpace == EAlignSpace::Local)
	{
		SetActorLocationAndRotation(InLocation, InRotation);
		if (ScaleGizmoGroup)
		{
			ScaleGizmoGroup->SetWorldRotation(InRotation);
		}
	}
	else
	{
		SetActorLocationAndRotation(InLocation, FRotator::ZeroRotator);
		if (ScaleGizmoGroup)
		{
			ScaleGizmoGroup->SetWorldRotation(InRotation);
		}
	}
}

bool ARuntimeTransformGizmo::IntersectRayConstraintPlane(const FVector& RayStart, const FVector& RayEnd, FVector& OutPoint) const
{
	const FVector PlaneNormal(DragConstraintPlane.X, DragConstraintPlane.Y, DragConstraintPlane.Z);
	const FVector Dir = (RayEnd - RayStart).GetSafeNormal();
	const float Denominator = FVector::DotProduct(PlaneNormal, Dir);
	if (FMath::IsNearlyZero(Denominator))
	{
		return false;
	}

	const float T = (DragConstraintPlane.W - FVector::DotProduct(PlaneNormal, RayStart)) / Denominator;
	OutPoint = RayStart + Dir * T;
	return true;
}

FVector ARuntimeTransformGizmo::GetViewLocation() const
{
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (const APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			return Cam->GetCameraLocation();
		}
	}
	return FVector::ZeroVector;
}

FRotator ARuntimeTransformGizmo::GetViewRotation() const
{
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (const APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			return Cam->GetCameraRotation();
		}
	}
	return FRotator::ZeroRotator;
}

FVector ARuntimeTransformGizmo::GetViewDirection() const
{
	return GetViewRotation().Vector();
}

FVector ARuntimeTransformGizmo::GetDirectionToView() const
{
	if (IsOrthographicView())
	{
		return GetViewDirection();
	}
	return (GetViewLocation() - WorldLocation).GetSafeNormal();
}

bool ARuntimeTransformGizmo::IsOrthographicView() const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return false;
	}

	if (const AActor* ViewTarget = PC->GetViewTarget())
	{
		if (const UCameraComponent* Camera = ViewTarget->FindComponentByClass<UCameraComponent>())
		{
			return Camera->ProjectionMode == ECameraProjectionMode::Orthographic;
		}
	}
	return false;
}

float ARuntimeTransformGizmo::GetOrthoWidth() const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC)
	{
		if (const AActor* ViewTarget = PC->GetViewTarget())
		{
			if (const UCameraComponent* Camera = ViewTarget->FindComponentByClass<UCameraComponent>())
			{
				return Camera->OrthoWidth;
			}
		}
	}
	return 2048.0f;
}

float ARuntimeTransformGizmo::GetFieldOfView() const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC)
	{
		if (const AActor* ViewTarget = PC->GetViewTarget())
		{
			if (const UCameraComponent* Camera = ViewTarget->FindComponentByClass<UCameraComponent>())
			{
				return Camera->FieldOfView;
			}
		}
	}
	return 90.0f;
}

FVector ARuntimeTransformGizmo::GetAxisVector(EConstraintAxis Axis, bool bAllowPlaneNormal) const
{
	switch (Axis)
	{
	case EConstraintAxis::X:
		return AlignRotator.RotateVector(FVector::ForwardVector);
	case EConstraintAxis::Y:
		return AlignRotator.RotateVector(FVector::RightVector);
	case EConstraintAxis::Z:
		return AlignRotator.RotateVector(FVector::UpVector);
	case EConstraintAxis::XY:
		return bAllowPlaneNormal ? AlignRotator.RotateVector(FVector::UpVector) : AlignRotator.RotateVector(FVector(1, 1, 0));
	case EConstraintAxis::XZ:
		return bAllowPlaneNormal ? AlignRotator.RotateVector(FVector::RightVector) : AlignRotator.RotateVector(FVector(1, 0, 1));
	case EConstraintAxis::YZ:
		return bAllowPlaneNormal ? AlignRotator.RotateVector(FVector::ForwardVector) : AlignRotator.RotateVector(FVector(0, 1, 1));
	case EConstraintAxis::XYZ:
		return AlignRotator.RotateVector(FVector(1, 1, 1));
	case EConstraintAxis::Screen:
	default:
		return GetDirectionToView();
	}
}

void ARuntimeTransformGizmo::ReleasePointerLock(UPointerInteractionComponent* PointerOverride)
{
	UPointerInteractionComponent* PointerToUnlock = PointerOverride ? PointerOverride : ActivePointer.Get();
	if (PointerToUnlock)
	{
		PointerToUnlock->SetLockHover(false);
	}
	ActivePointer = nullptr;
}

void ARuntimeTransformGizmo::DumpHandleRelativeTransforms() const
{
	auto DumpOne = [](const UStaticMeshComponent* Comp, const TCHAR* Label)
	{
		if (!Comp)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GizmoLayout] %s: <null>"), Label);
			return;
		}

		const FVector L = Comp->GetRelativeLocation();
		const FRotator R = Comp->GetRelativeRotation();
		const FVector S = Comp->GetRelativeScale3D();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GizmoLayout] %s | Loc=(%.3f, %.3f, %.3f) Rot=(%.3f, %.3f, %.3f) Scale=(%.3f, %.3f, %.3f)"),
			Label, L.X, L.Y, L.Z, R.Pitch, R.Yaw, R.Roll, S.X, S.Y, S.Z
		);
	};

	UE_LOG(LogTemp, Warning, TEXT("[GizmoLayout] ===== Dump Start: %s ====="), *GetName());
	DumpOne(TranslationHandle_X, TEXT("TranslationHandle_X"));
	DumpOne(TranslationHandle_Y, TEXT("TranslationHandle_Y"));
	DumpOne(TranslationHandle_Z, TEXT("TranslationHandle_Z"));
	DumpOne(PlaneTranslationHandle_XY, TEXT("PlaneTranslationHandle_XY"));
	DumpOne(PlaneTranslationHandle_XZ, TEXT("PlaneTranslationHandle_XZ"));
	DumpOne(PlaneTranslationHandle_YZ, TEXT("PlaneTranslationHandle_YZ"));
	DumpOne(ScreenTranslationHandle, TEXT("ScreenTranslationHandle"));

	DumpOne(RotationHandle_X, TEXT("RotationHandle_X"));
	DumpOne(RotationHandle_Y, TEXT("RotationHandle_Y"));
	DumpOne(RotationHandle_Z, TEXT("RotationHandle_Z"));
	DumpOne(TrackballRotationHandle, TEXT("TrackballRotationHandle"));
	DumpOne(ScreenRotationHandle, TEXT("ScreenRotationHandle"));

	DumpOne(ScaleHandle_X, TEXT("ScaleHandle_X"));
	DumpOne(ScaleHandle_Y, TEXT("ScaleHandle_Y"));
	DumpOne(ScaleHandle_Z, TEXT("ScaleHandle_Z"));
	DumpOne(PlaneScaleHandle_XY, TEXT("PlaneScaleHandle_XY"));
	DumpOne(PlaneScaleHandle_XZ, TEXT("PlaneScaleHandle_XZ"));
	DumpOne(PlaneScaleHandle_YZ, TEXT("PlaneScaleHandle_YZ"));
	DumpOne(UniformScaleHandle, TEXT("UniformScaleHandle"));
	UE_LOG(LogTemp, Warning, TEXT("[GizmoLayout] ===== Dump End: %s ====="), *GetName());
}

void ARuntimeTransformGizmo::SetupDefaultHandleTransforms()
{
	if (TranslationHandle_X)
	{
		TranslationHandle_X->SetRelativeLocation(FVector::ZeroVector);
		TranslationHandle_X->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (TranslationHandle_Y)
	{
		TranslationHandle_Y->SetRelativeLocation(FVector::ZeroVector);
		TranslationHandle_Y->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
	if (TranslationHandle_Z)
	{
		TranslationHandle_Z->SetRelativeLocation(FVector::ZeroVector);
		TranslationHandle_Z->SetRelativeRotation(FRotator(90.0f, 0.0f, 90.0f));
	}
	if (PlaneTranslationHandle_XY)
	{
		PlaneTranslationHandle_XY->SetRelativeLocation(FVector::ZeroVector);
		PlaneTranslationHandle_XY->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}
	if (PlaneTranslationHandle_XZ)
	{
		PlaneTranslationHandle_XZ->SetRelativeLocation(FVector::ZeroVector);
		PlaneTranslationHandle_XZ->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
	if (PlaneTranslationHandle_YZ)
	{
		PlaneTranslationHandle_YZ->SetRelativeLocation(FVector::ZeroVector);
		PlaneTranslationHandle_YZ->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (ScreenTranslationHandle)
	{
		ScreenTranslationHandle->SetRelativeLocation(FVector::ZeroVector);
		ScreenTranslationHandle->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (RotationHandle_X)
	{
		RotationHandle_X->SetRelativeLocation(FVector::ZeroVector);
		RotationHandle_X->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	}
	if (RotationHandle_Y)
	{
		RotationHandle_Y->SetRelativeLocation(FVector::ZeroVector);
		RotationHandle_Y->SetRelativeRotation(FRotator(0.0f, -90.0f, -90.0f));
	}
	if (RotationHandle_Z)
	{
		RotationHandle_Z->SetRelativeLocation(FVector::ZeroVector);
		RotationHandle_Z->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	}
	if (TrackballRotationHandle)
	{
		TrackballRotationHandle->SetRelativeLocation(FVector::ZeroVector);
		TrackballRotationHandle->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (ScreenRotationHandle)
	{
		ScreenRotationHandle->SetRelativeLocation(FVector::ZeroVector);
		ScreenRotationHandle->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (ScaleHandle_X)
	{
		ScaleHandle_X->SetRelativeLocation(FVector::ZeroVector);
		ScaleHandle_X->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (ScaleHandle_Y)
	{
		ScaleHandle_Y->SetRelativeLocation(FVector::ZeroVector);
		ScaleHandle_Y->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
	if (ScaleHandle_Z)
	{
		ScaleHandle_Z->SetRelativeLocation(FVector::ZeroVector);
		ScaleHandle_Z->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	}
	if (PlaneScaleHandle_XY)
	{
		PlaneScaleHandle_XY->SetRelativeLocation(FVector::ZeroVector);
		PlaneScaleHandle_XY->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}
	if (PlaneScaleHandle_XZ)
	{
		PlaneScaleHandle_XZ->SetRelativeLocation(FVector::ZeroVector);
		PlaneScaleHandle_XZ->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
	if (PlaneScaleHandle_YZ)
	{
		PlaneScaleHandle_YZ->SetRelativeLocation(FVector::ZeroVector);
		PlaneScaleHandle_YZ->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (UniformScaleHandle)
	{
		UniformScaleHandle->SetRelativeLocation(FVector::ZeroVector);
		UniformScaleHandle->SetRelativeRotation(FRotator::ZeroRotator);
	}
}
