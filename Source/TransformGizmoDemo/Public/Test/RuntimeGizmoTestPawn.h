#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "RuntimeGizmoTestPawn.generated.h"

class ARuntimeTransformGizmo;
class UPointerInteractionComponent;

UCLASS(Blueprintable)
class TRANSFORMGIZMODEMO_API ARuntimeGizmoTestPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	ARuntimeGizmoTestPawn();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UFUNCTION()
	void OnLeftMousePressed();

	UFUNCTION()
	void OnLeftMouseReleased();

	UFUNCTION()
	void OnRightMousePressed();

	UFUNCTION()
	void OnRightMouseReleased();

	UFUNCTION()
	void OnKeyW();

	UFUNCTION()
	void OnKeyE();

	UFUNCTION()
	void OnKeyR();

	UFUNCTION()
	void OnKeyT();

	UFUNCTION()
	void OnKeyQ();

	UFUNCTION()
	void HandleGizmoTransformUpdated(const FTransform& Transform);

private:
	void TrySelectActorUnderPointer();
	APlayerController* GetPC() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pointer")
	TObjectPtr<UPointerInteractionComponent> PointerInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo")
	TObjectPtr<ARuntimeTransformGizmo> Gizmo = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo")
	TObjectPtr<AActor> SelectedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bRightMousePressed = false;
};
