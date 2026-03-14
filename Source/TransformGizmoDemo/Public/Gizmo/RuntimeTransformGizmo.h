#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gizmo/RuntimeTransformGizmoTypes.h"
#include "Pointer/PointerInteractableInterface.h"
#include "RuntimeTransformGizmo.generated.h"

class UPointerInteractionComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGizmoTransformEvent, const FTransform&, Transform);

UCLASS(BlueprintType, Blueprintable)
class TRANSFORMGIZMODEMO_API ARuntimeTransformGizmo : public AActor, public IPointerInteractableInterface
{
	GENERATED_BODY()

public:
	ARuntimeTransformGizmo();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void ActivateGizmo(const FTransform& InitialTransform);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void DeactivateGizmo();

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SetTransformMode(ETransformMode InTransformMode, bool bInCombineTranslateRotate = false);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SetAlignSpace(EAlignSpace InAlignSpace);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SetTransformValues(const FVector& InLocation, const FRotator& InRotation, const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SetActiveGizmoGroup(bool bTranslation, bool bRotation, bool bScale);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void InitializeGizmoHandleMap();

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void UpdateGizmoScale();

	UFUNCTION(BlueprintCallable, Category = "Gizmo|Debug")
	void DumpHandleRelativeTransforms() const;

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	FVector GetSnappedLocation(const FVector& LocationDelta) const;

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	float GetSnappedAngle(float Angle) const;

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	float GetSnappedScale(float ScaleDelta) const;

	UPROPERTY(BlueprintAssignable, Category = "Gizmo|Events")
	FGizmoTransformEvent OnBeginTransform;

	UPROPERTY(BlueprintAssignable, Category = "Gizmo|Events")
	FGizmoTransformEvent OnUpdateTransform;

	UPROPERTY(BlueprintAssignable, Category = "Gizmo|Events")
	FGizmoTransformEvent OnFinishTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	int32 BaseInteractionPriority = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	float GizmoScreenSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bScaledByDPI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bUseDefaultHandleLayout = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Debug")
	bool bDumpHandleTransformsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bEnableLocationSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bEnableRotationSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bEnableScaleSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings", meta = (ClampMin = "0.001"))
	float LocationGridSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings", meta = (ClampMin = "0.001"))
	float RotationGridSize = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings", meta = (ClampMin = "0.001"))
	float ScaleGridSize = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	EAlignSpace AlignSpace = EAlignSpace::World;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	ETransformMode ActiveTransformMode = ETransformMode::Translation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	bool bCombineTranslateRotate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	float RotateFactor = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo|Settings")
	float ScaleFactor = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	bool bIsActivated = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	bool bIsDragging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	FVector WorldScale = FVector::OneVector;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo|State")
	float GizmoWorldScale = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<USceneComponent> TransformGizmoGroup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<USceneComponent> TranslationGizmoGroup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<USceneComponent> RotationGizmoGroup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<USceneComponent> ScaleGizmoGroup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> TranslationHandle_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> TranslationHandle_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> TranslationHandle_Z;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneTranslationHandle_XY;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneTranslationHandle_XZ;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneTranslationHandle_YZ;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> ScreenTranslationHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> RotationHandle_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> RotationHandle_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> RotationHandle_Z;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> TrackballRotationHandle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> ScreenRotationHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> ScaleHandle_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> ScaleHandle_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> ScaleHandle_Z;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneScaleHandle_XY;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneScaleHandle_XZ;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> PlaneScaleHandle_YZ;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo|Components")
	TObjectPtr<UStaticMeshComponent> UniformScaleHandle;

	virtual int32 GetHitPriority_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const override;
	virtual bool IgnoreHitTest_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const override;
	virtual bool OnPointerDown_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;
	virtual void OnPointerDrag_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;
	virtual void OnPointerEnter_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;
	virtual void OnPointerLeave_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;
	virtual void OnPointerMove_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;
	virtual void OnPointerUp_Implementation(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) override;

private:
	UStaticMeshComponent* CreateHandle(const TCHAR* Name, USceneComponent* Parent);
	void RegisterHandle(UPrimitiveComponent* Component, const FGizmoHandleInfo& Info);
	bool BuildDragContext(UPrimitiveComponent* Handle, const FVector& RayStart, const FVector& RayEnd);
	void ApplyGizmoTransform(const FVector& InLocation, const FRotator& InRotation, const FVector& InScale);
	bool IntersectRayConstraintPlane(const FVector& RayStart, const FVector& RayEnd, FVector& OutPoint) const;
	FVector GetViewLocation() const;
	FRotator GetViewRotation() const;
	FVector GetViewDirection() const;
	FVector GetDirectionToView() const;
	bool IsOrthographicView() const;
	float GetOrthoWidth() const;
	float GetFieldOfView() const;
	FVector GetAxisVector(EConstraintAxis Axis, bool bAllowPlaneNormal = false) const;
	void SetupDefaultHandleTransforms();
	void ReleasePointerLock(UPointerInteractionComponent* PointerOverride = nullptr);

private:
	TMap<TObjectPtr<UPrimitiveComponent>, FGizmoHandleInfo> GizmoHandleMap;
	TWeakObjectPtr<UPointerInteractionComponent> ActivePointer;
	FPlane DragConstraintPlane = FPlane(FVector::ZeroVector, FVector::UpVector);
	FGizmoHandleInfo ActiveDragHandleInfo;
	TObjectPtr<UPrimitiveComponent> ActiveDragHandle = nullptr;
	FVector StartDragLocation = FVector::ZeroVector;
	FVector LastDragLocation = FVector::ZeroVector;
	FVector WorldLocationOnDragStart = FVector::ZeroVector;
	FRotator WorldRotationOnDragStart = FRotator::ZeroRotator;
	FVector WorldScaleOnDragStart = FVector::OneVector;
	FRotator AlignRotator = FRotator::ZeroRotator;
	float RotationAngle = 0.0f;
	float DraggedAngle = 0.0f;
};
