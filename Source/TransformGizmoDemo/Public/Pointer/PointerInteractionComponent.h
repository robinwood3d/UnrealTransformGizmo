#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Pointer/PointerInteractionTypes.h"
#include "PointerInteractionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSFORMGIZMODEMO_API UPointerInteractionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPointerInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void SetupInteraction(const FPointerInteractionSettings& NewSettings, bool bActivate);

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void EnableInteraction();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void DisableInteraction();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void PressPointer();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void ReleasePointer();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void DragPointer();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void UpdatePointerHover();

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	FHitResult RaycastTest() const;

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void SetCustomHitResult(const FHitResult& InHitResult);

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	FHitResult GetLastHitResult() const;

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void GetHoveredObject(AActor*& OutActor, UPrimitiveComponent*& OutComponent) const;

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void SetLockHover(bool bInLock);

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	void GetPointerRay(FVector& OutRayStart, FVector& OutRayEnd) const;

	UFUNCTION(BlueprintCallable, Category = "Pointer")
	bool CanInteractWith(AActor* Actor, UPrimitiveComponent* Component) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	FPointerInteractionSettings InteractionSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(BlueprintReadOnly, Category = "Pointer")
	FHitResult SavedHitResult;

	UPROPERTY(BlueprintReadOnly, Category = "Pointer")
	bool bIsDragging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pointer")
	bool bIsHoverLocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pointer")
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Pointer")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

private:
	APlayerController* ResolvePlayerController() const;
	FCollisionObjectQueryParams MakeObjectQueryParams() const;
	FCollisionQueryParams MakeQueryParams() const;
	bool SelectBestHit(const TArray<FHitResult>& Hits, FHitResult& OutHit) const;
};
