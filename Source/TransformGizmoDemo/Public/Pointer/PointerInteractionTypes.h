#pragma once

#include "CoreMinimal.h"
#include "PointerInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EPointerInteractionSource : uint8
{
	MouseCursor UMETA(DisplayName = "Mouse Cursor"),
	ScreenCenter UMETA(DisplayName = "Screen Center"),
	World UMETA(DisplayName = "World"),
	Custom UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FPointerInteractionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	EPointerInteractionSource InteractionSource = EPointerInteractionSource::MouseCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	float InteractionDistance = 1000000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	bool bHitComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	bool bUseMultiRayHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	float RayWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	bool bEnableDragging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	bool bHoverOnPress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	TArray<TEnumAsByte<EObjectTypeQuery>> InteractableTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	TArray<TObjectPtr<AActor>> IgnoredActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	TArray<TObjectPtr<AActor>> OnlyActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	TObjectPtr<UPrimitiveComponent> OverlapTestShape = nullptr;
};
