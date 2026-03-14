#pragma once

#include "CoreMinimal.h"
#include "RuntimeTransformGizmoTypes.generated.h"

UENUM(BlueprintType)
enum class ETransformMode : uint8
{
	Translation,
	Rotation,
	Scale
};

UENUM(BlueprintType)
enum class EAlignSpace : uint8
{
	World,
	Local
};

UENUM(BlueprintType)
enum class EConstraintAxis : uint8
{
	Screen,
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ,
	XYZ
};

USTRUCT(BlueprintType)
struct FGizmoHandleInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo")
	ETransformMode TransformMode = ETransformMode::Translation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo")
	EConstraintAxis ConstraintAxis = EConstraintAxis::Screen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo")
	int32 HitPriority = 0;
};
