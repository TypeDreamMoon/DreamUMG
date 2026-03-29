// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/DreamCurvedBox.h"

#include "Widgets/SDreamCurvedBox.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DreamCurvedBox"

UDreamCurvedBox::UDreamCurvedBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	Rotation = FRotator::ZeroRotator;
	CurveAngle = 45.0f;
	CurveSegments = 32;
	PerspectiveStrength = 0.35f;
	DepthOffset = 0.0f;
	TransformPivot = FVector2D(0.5f, 0.5f);
}

void UDreamCurvedBox::SetRotation(FRotator InRotation)
{
	Rotation = InRotation;
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetRotation(Rotation);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SetCurveAngle(float InCurveAngle)
{
	CurveAngle = InCurveAngle;
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetCurveAngle(CurveAngle);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SetCurveSegments(int32 InCurveSegments)
{
	CurveSegments = FMath::Clamp(InCurveSegments, 2, 128);
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetCurveSegments(CurveSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	PerspectiveStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetPerspectiveStrength(PerspectiveStrength);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SetDepthOffset(float InDepthOffset)
{
	DepthOffset = InDepthOffset;
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetDepthOffset(DepthOffset);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SetTransformPivot(FVector2D InTransformPivot)
{
	TransformPivot.X = FMath::Clamp(InTransformPivot.X, 0.0f, 1.0f);
	TransformPivot.Y = FMath::Clamp(InTransformPivot.Y, 0.0f, 1.0f);
	if (MyCurvedBox.IsValid())
	{
		MyCurvedBox->SetTransformPivot(TransformPivot);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamCurvedBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushAllPropertiesToSlate();
	RefreshContent();
}

void UDreamCurvedBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyCurvedBox.Reset();
}

#if WITH_EDITOR
const FText UDreamCurvedBox::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}
#endif

TSharedRef<SWidget> UDreamCurvedBox::RebuildWidget()
{
	SAssignNew(MyCurvedBox, SDreamCurvedBox);
	PushAllPropertiesToSlate();
	RefreshContent();
	return MyCurvedBox.ToSharedRef();
}

void UDreamCurvedBox::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
	RefreshContent();
}

void UDreamCurvedBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	Super::OnSlotRemoved(InSlot);
	RefreshContent();
}

void UDreamCurvedBox::PushAllPropertiesToSlate()
{
	if (!MyCurvedBox.IsValid())
	{
		return;
	}

	MyCurvedBox->SetRotation(Rotation);
	MyCurvedBox->SetCurveAngle(CurveAngle);
	MyCurvedBox->SetCurveSegments(CurveSegments);
	MyCurvedBox->SetPerspectiveStrength(PerspectiveStrength);
	MyCurvedBox->SetDepthOffset(DepthOffset);
	MyCurvedBox->SetTransformPivot(TransformPivot);
}

void UDreamCurvedBox::RefreshContent()
{
	if (!MyCurvedBox.IsValid())
	{
		return;
	}

	if (UWidget* Content = GetContent())
	{
		MyCurvedBox->SetContent(Content->TakeWidget());
	}
	else
	{
		MyCurvedBox->SetContent(SNullWidget::NullWidget);
	}
}

#undef LOCTEXT_NAMESPACE
