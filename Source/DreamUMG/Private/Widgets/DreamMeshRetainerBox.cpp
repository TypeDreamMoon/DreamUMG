// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/DreamMeshRetainerBox.h"

#include "Widgets/SDreamMeshRetainerBox.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DreamMeshRetainerBox"

UDreamMeshRetainerBox::UDreamMeshRetainerBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::Visible);

	Rotation = FRotator::ZeroRotator;
	CurveAngle = 45.0f;
	CurveSegments = 48;
	VerticalSegments = 1;
	PerspectiveStrength = 0.35f;
	DepthOffset = 0.0f;
	TransformPivot = FVector2D(0.5f, 0.5f);
	RetainedRenderScale = 1.0f;
	bRenderEveryFrame = true;
}

void UDreamMeshRetainerBox::SetRotation(FRotator InRotation)
{
	Rotation = InRotation;
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetRotation(Rotation);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetCurveAngle(float InCurveAngle)
{
	CurveAngle = InCurveAngle;
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetCurveAngle(CurveAngle);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetCurveSegments(int32 InCurveSegments)
{
	CurveSegments = FMath::Clamp(InCurveSegments, 2, 256);
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetCurveSegments(CurveSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetVerticalSegments(int32 InVerticalSegments)
{
	VerticalSegments = FMath::Clamp(InVerticalSegments, 1, 64);
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetVerticalSegments(VerticalSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	PerspectiveStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetPerspectiveStrength(PerspectiveStrength);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetDepthOffset(float InDepthOffset)
{
	DepthOffset = InDepthOffset;
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetDepthOffset(DepthOffset);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetTransformPivot(FVector2D InTransformPivot)
{
	TransformPivot.X = FMath::Clamp(InTransformPivot.X, 0.0f, 1.0f);
	TransformPivot.Y = FMath::Clamp(InTransformPivot.Y, 0.0f, 1.0f);
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetTransformPivot(TransformPivot);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetRetainedRenderScale(float InRetainedRenderScale)
{
	RetainedRenderScale = FMath::Max(0.1f, InRetainedRenderScale);
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetRetainedRenderScale(RetainedRenderScale);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamMeshRetainerBox::SetRenderEveryFrame(bool bInRenderEveryFrame)
{
	bRenderEveryFrame = bInRenderEveryFrame;
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->SetRenderEveryFrame(bRenderEveryFrame);
	}
}

void UDreamMeshRetainerBox::RequestRender()
{
	if (MyMeshRetainerBox.IsValid())
	{
		MyMeshRetainerBox->RequestRender();
	}
}

void UDreamMeshRetainerBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushAllPropertiesToSlate();
	RefreshContent();
}

void UDreamMeshRetainerBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyMeshRetainerBox.Reset();
}

#if WITH_EDITOR
const FText UDreamMeshRetainerBox::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}
#endif

TSharedRef<SWidget> UDreamMeshRetainerBox::RebuildWidget()
{
	SAssignNew(MyMeshRetainerBox, SDreamMeshRetainerBox);
	PushAllPropertiesToSlate();
	RefreshContent();
	return MyMeshRetainerBox.ToSharedRef();
}

void UDreamMeshRetainerBox::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
	RefreshContent();
}

void UDreamMeshRetainerBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	Super::OnSlotRemoved(InSlot);
	RefreshContent();
}

void UDreamMeshRetainerBox::PushAllPropertiesToSlate()
{
	if (!MyMeshRetainerBox.IsValid())
	{
		return;
	}

	MyMeshRetainerBox->SetRotation(Rotation);
	MyMeshRetainerBox->SetCurveAngle(CurveAngle);
	MyMeshRetainerBox->SetCurveSegments(CurveSegments);
	MyMeshRetainerBox->SetVerticalSegments(VerticalSegments);
	MyMeshRetainerBox->SetPerspectiveStrength(PerspectiveStrength);
	MyMeshRetainerBox->SetDepthOffset(DepthOffset);
	MyMeshRetainerBox->SetTransformPivot(TransformPivot);
	MyMeshRetainerBox->SetRetainedRenderScale(RetainedRenderScale);
	MyMeshRetainerBox->SetRenderEveryFrame(bRenderEveryFrame);
}

void UDreamMeshRetainerBox::RefreshContent()
{
	if (!MyMeshRetainerBox.IsValid())
	{
		return;
	}

	if (UWidget* Content = GetContent())
	{
		MyMeshRetainerBox->SetContent(Content->TakeWidget());
	}
	else
	{
		MyMeshRetainerBox->SetContent(SNullWidget::NullWidget);
	}
}

#undef LOCTEXT_NAMESPACE
