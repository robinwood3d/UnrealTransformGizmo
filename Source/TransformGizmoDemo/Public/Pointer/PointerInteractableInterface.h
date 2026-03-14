#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PointerInteractableInterface.generated.h"

class UPrimitiveComponent;
class UPointerInteractionComponent;

UINTERFACE(BlueprintType)
class TRANSFORMGIZMODEMO_API UPointerInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class TRANSFORMGIZMODEMO_API IPointerInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	int32 GetHitPriority(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	bool IgnoreHitTest(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	bool OnPointerDown(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	void OnPointerDrag(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	void OnPointerEnter(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	void OnPointerLeave(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	void OnPointerMove(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pointer")
	void OnPointerUp(UPointerInteractionComponent* Pointer, UPrimitiveComponent* Component);
};
