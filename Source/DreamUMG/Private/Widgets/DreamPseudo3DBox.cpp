// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/DreamPseudo3DBox.h"

#include "Widgets/SDreamPseudo3DBox.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DreamPseudo3DBox"

UDreamPseudo3DBox::UDreamPseudo3DBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	Rotation = FRotator::ZeroRotator;
	PerspectiveStrength = 0.35f;
	FieldOfView = 45.0f;
	DepthOffset = 0.0f;
	PerspectiveSegments = 6;
	TransformPivot = FVector2D(0.5f, 0.5f);
	bIgnoreClipping = true;
}

void UDreamPseudo3DBox::SetRotation(FRotator InRotation)
{
	Rotation = InRotation;
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetRotation(Rotation);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	PerspectiveStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetPerspectiveStrength(PerspectiveStrength);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetFieldOfView(float InFieldOfView)
{
	FieldOfView = FMath::Clamp(InFieldOfView, 1.0f, 170.0f);
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetFieldOfView(FieldOfView);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetDepthOffset(float InDepthOffset)
{
	DepthOffset = InDepthOffset;
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetDepthOffset(DepthOffset);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetPerspectiveSegments(int32 InPerspectiveSegments)
{
	PerspectiveSegments = FMath::Clamp(InPerspectiveSegments, 1, 12);
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetPerspectiveSegments(PerspectiveSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetTransformPivot(FVector2D InTransformPivot)
{
	TransformPivot.X = FMath::Clamp(InTransformPivot.X, 0.0f, 1.0f);
	TransformPivot.Y = FMath::Clamp(InTransformPivot.Y, 0.0f, 1.0f);
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetTransformPivot(TransformPivot);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SetIgnoreClipping(bool bInIgnoreClipping)
{
	bIgnoreClipping = bInIgnoreClipping;
	if (MyPseudo3DBox.IsValid())
	{
		MyPseudo3DBox->SetIgnoreClipping(bIgnoreClipping);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamPseudo3DBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushAllPropertiesToSlate();
	RefreshContent();
}

void UDreamPseudo3DBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyPseudo3DBox.Reset();
}

#if WITH_EDITOR
const FText UDreamPseudo3DBox::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}
#endif

TSharedRef<SWidget> UDreamPseudo3DBox::RebuildWidget()
{
	SAssignNew(MyPseudo3DBox, SDreamPseudo3DBox);
	PushAllPropertiesToSlate();
	RefreshContent();
	return MyPseudo3DBox.ToSharedRef();
}

void UDreamPseudo3DBox::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
	RefreshContent();
}

void UDreamPseudo3DBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	Super::OnSlotRemoved(InSlot);
	RefreshContent();
}

void UDreamPseudo3DBox::PushAllPropertiesToSlate()
{
	if (!MyPseudo3DBox.IsValid())
	{
		return;
	}

	MyPseudo3DBox->SetRotation(Rotation);
	MyPseudo3DBox->SetPerspectiveStrength(PerspectiveStrength);
	MyPseudo3DBox->SetFieldOfView(FieldOfView);
	MyPseudo3DBox->SetDepthOffset(DepthOffset);
	MyPseudo3DBox->SetPerspectiveSegments(PerspectiveSegments);
	MyPseudo3DBox->SetTransformPivot(TransformPivot);
	MyPseudo3DBox->SetIgnoreClipping(bIgnoreClipping);
}

void UDreamPseudo3DBox::RefreshContent()
{
	if (!MyPseudo3DBox.IsValid())
	{
		return;
	}

	if (UWidget* Content = GetContent())
	{
		MyPseudo3DBox->SetContent(Content->TakeWidget());
	}
	else
	{
		MyPseudo3DBox->SetContent(SNullWidget::NullWidget);
	}
}

#undef LOCTEXT_NAMESPACE
