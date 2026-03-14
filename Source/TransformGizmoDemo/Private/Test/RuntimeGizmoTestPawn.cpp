#include "Test/RuntimeGizmoTestPawn.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Gizmo/RuntimeTransformGizmo.h"
#include "InputCoreTypes.h"
#include "Pointer/PointerInteractionComponent.h"
#include "Pointer/PointerInteractionTypes.h"

ARuntimeGizmoTestPawn::ARuntimeGizmoTestPawn()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	PointerInteraction = CreateDefaultSubobject<UPointerInteractionComponent>(TEXT("PointerInteraction"));
	PointerInteraction->SetupAttachment(GetRootComponent());
}

void ARuntimeGizmoTestPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetPC())
	{
		PC->SetShowMouseCursor(true);
		PC->SetIgnoreMoveInput(true);
	}

	if (!Gizmo)
	{
		for (TActorIterator<ARuntimeTransformGizmo> It(GetWorld()); It; ++It)
		{
			Gizmo = *It;
			break;
		}
	}

	if (Gizmo)
	{
		Gizmo->OnUpdateTransform.RemoveDynamic(this, &ARuntimeGizmoTestPawn::HandleGizmoTransformUpdated);
		Gizmo->OnUpdateTransform.AddDynamic(this, &ARuntimeGizmoTestPawn::HandleGizmoTransformUpdated);
		Gizmo->DeactivateGizmo();
	}

	FPointerInteractionSettings Settings;
	Settings.InteractionSource = EPointerInteractionSource::MouseCursor;
	Settings.InteractionDistance = 100000000.0f;
	Settings.bHitComplex = true;
	Settings.bUseMultiRayHit = true;
	Settings.InteractableTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	PointerInteraction->SetupInteraction(Settings, true);
}

void ARuntimeGizmoTestPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnLeftMousePressed);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ARuntimeGizmoTestPawn::OnLeftMouseReleased);

	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnRightMousePressed);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ARuntimeGizmoTestPawn::OnRightMouseReleased);

	PlayerInputComponent->BindKey(EKeys::W, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnKeyW);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnKeyE);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnKeyR);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnKeyT);
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ARuntimeGizmoTestPawn::OnKeyQ);
}

void ARuntimeGizmoTestPawn::OnLeftMousePressed()
{
	AActor* HoveredActor = nullptr;
	UPrimitiveComponent* HoveredComponent = nullptr;
	PointerInteraction->GetHoveredObject(HoveredActor, HoveredComponent);

	if (HoveredActor)
	{
		PointerInteraction->PressPointer();
		if (APlayerController* PC = GetPC())
		{
			PC->SetIgnoreLookInput(true);
		}
	}
}

void ARuntimeGizmoTestPawn::OnLeftMouseReleased()
{
	const bool bWasDragging = PointerInteraction && PointerInteraction->bIsDragging;
	if (bWasDragging)
	{
		PointerInteraction->ReleasePointer();
	}

	if (APlayerController* PC = GetPC())
	{
		PC->SetIgnoreLookInput(false);
	}

	if (!bWasDragging)
	{
		TrySelectActorUnderPointer();
	}
}

void ARuntimeGizmoTestPawn::OnRightMousePressed()
{
	bRightMousePressed = true;
	if (APlayerController* PC = GetPC())
	{
		PC->SetIgnoreMoveInput(false);
	}
}

void ARuntimeGizmoTestPawn::OnRightMouseReleased()
{
	bRightMousePressed = false;
	if (APlayerController* PC = GetPC())
	{
		PC->SetIgnoreMoveInput(true);
	}
}

void ARuntimeGizmoTestPawn::OnKeyW()
{
	if (!bRightMousePressed && Gizmo)
	{
		Gizmo->SetTransformMode(ETransformMode::Translation, false);
	}
}

void ARuntimeGizmoTestPawn::OnKeyE()
{
	if (!bRightMousePressed && Gizmo)
	{
		Gizmo->SetTransformMode(ETransformMode::Rotation, false);
	}
}

void ARuntimeGizmoTestPawn::OnKeyR()
{
	if (!bRightMousePressed && Gizmo)
	{
		Gizmo->SetTransformMode(ETransformMode::Scale, false);
	}
}

void ARuntimeGizmoTestPawn::OnKeyT()
{
	if (!bRightMousePressed && Gizmo)
	{
		Gizmo->SetTransformMode(ETransformMode::Translation, true);
	}
}

void ARuntimeGizmoTestPawn::OnKeyQ()
{
	if (!bRightMousePressed && Gizmo)
	{
		const EAlignSpace NewSpace = (Gizmo->AlignSpace == EAlignSpace::World) ? EAlignSpace::Local : EAlignSpace::World;
		Gizmo->SetAlignSpace(NewSpace);
	}
}

void ARuntimeGizmoTestPawn::HandleGizmoTransformUpdated(const FTransform& Transform)
{
	if (!SelectedActor)
	{
		return;
	}

	USceneComponent* SelectedRootComponent = SelectedActor->GetRootComponent();
	if (!SelectedRootComponent)
	{
		return;
	}

	const EComponentMobility::Type OldMobility = SelectedRootComponent->Mobility;
	if (OldMobility != EComponentMobility::Movable)
	{
		SelectedRootComponent->SetMobility(EComponentMobility::Movable);
	}

	SelectedActor->SetActorTransform(Transform);

	if (SelectedRootComponent->Mobility != OldMobility)
	{
		SelectedRootComponent->SetMobility(OldMobility);
	}
}

void ARuntimeGizmoTestPawn::TrySelectActorUnderPointer()
{
	if (!Gizmo || !GetWorld())
	{
		return;
	}

	FVector RayStart;
	FVector RayEnd;
	PointerInteraction->GetPointerRay(RayStart, RayEnd);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TestPawnSelect), true);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Gizmo);

	if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_Visibility, Params) && Hit.GetActor())
	{
		SelectedActor = Hit.GetActor();
		Gizmo->ActivateGizmo(SelectedActor->GetActorTransform());
		return;
	}

	SelectedActor = nullptr;
	Gizmo->DeactivateGizmo();
}

APlayerController* ARuntimeGizmoTestPawn::GetPC() const
{
	return Cast<APlayerController>(GetController());
}
