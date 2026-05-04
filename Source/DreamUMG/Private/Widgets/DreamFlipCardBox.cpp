// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/DreamFlipCardBox.h"

#include "Components/PanelSlot.h"
#include "Widgets/SDreamFlipCardBox.h"

#define LOCTEXT_NAMESPACE "DreamFlipCardBox"

UDreamFlipCardBox::UDreamFlipCardBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	bCanHaveMultipleChildren = true;
	SetVisibilityInternal(ESlateVisibility::Visible);

	FlipAngle = 0.0f;
	HorizontalSegments = 24;
	VerticalSegments = 1;
	PerspectiveStrength = 0.35f;
	DepthOffset = 0.0f;
	TransformPivot = FVector2D(0.5f, 0.5f);
	RetainedRenderScale = 1.0f;
	bRenderEveryFrame = true;
	bMirrorBackFace = true;
}

void UDreamFlipCardBox::SetFlipAngle(float InFlipAngle)
{
	FlipAngle = InFlipAngle;
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetFlipAngle(FlipAngle);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetHorizontalSegments(int32 InHorizontalSegments)
{
	HorizontalSegments = FMath::Clamp(InHorizontalSegments, 1, 128);
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetHorizontalSegments(HorizontalSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetVerticalSegments(int32 InVerticalSegments)
{
	VerticalSegments = FMath::Clamp(InVerticalSegments, 1, 64);
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetVerticalSegments(VerticalSegments);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	PerspectiveStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetPerspectiveStrength(PerspectiveStrength);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetDepthOffset(float InDepthOffset)
{
	DepthOffset = InDepthOffset;
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetDepthOffset(DepthOffset);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetTransformPivot(FVector2D InTransformPivot)
{
	TransformPivot.X = FMath::Clamp(InTransformPivot.X, 0.0, 1.0);
	TransformPivot.Y = FMath::Clamp(InTransformPivot.Y, 0.0, 1.0);
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetTransformPivot(TransformPivot);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetRetainedRenderScale(float InRetainedRenderScale)
{
	RetainedRenderScale = FMath::Max(0.1f, InRetainedRenderScale);
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetRetainedRenderScale(RetainedRenderScale);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::SetRenderEveryFrame(bool bInRenderEveryFrame)
{
	bRenderEveryFrame = bInRenderEveryFrame;
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetRenderEveryFrame(bRenderEveryFrame);
	}
}

void UDreamFlipCardBox::SetMirrorBackFace(bool bInMirrorBackFace)
{
	bMirrorBackFace = bInMirrorBackFace;
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->SetMirrorBackFace(bMirrorBackFace);
	}
	InvalidateLayoutAndVolatility();
}

void UDreamFlipCardBox::RequestRender()
{
	if (MyFlipCardBox.IsValid())
	{
		MyFlipCardBox->RequestRender();
	}
}

UWidget* UDreamFlipCardBox::GetFrontWidget() const
{
	return GetChildAt(0);
}

UWidget* UDreamFlipCardBox::GetBackWidget() const
{
	return GetChildAt(1);
}

void UDreamFlipCardBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushAllPropertiesToSlate();
	RefreshChildren();
}

void UDreamFlipCardBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyFlipCardBox.Reset();
}

#if WITH_EDITOR
const FText UDreamFlipCardBox::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}
#endif

TSharedRef<SWidget> UDreamFlipCardBox::RebuildWidget()
{
	SAssignNew(MyFlipCardBox, SDreamFlipCardBox);
	MyFlipCardBox->SetOnHovered(FSimpleDelegate::CreateUObject(this, &UDreamFlipCardBox::HandleSlateHovered));
	MyFlipCardBox->SetOnUnhovered(FSimpleDelegate::CreateUObject(this, &UDreamFlipCardBox::HandleSlateUnhovered));
	PushAllPropertiesToSlate();
	RefreshChildren();
	return MyFlipCardBox.ToSharedRef();
}

UClass* UDreamFlipCardBox::GetSlotClass() const
{
	return UPanelSlot::StaticClass();
}

void UDreamFlipCardBox::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
	RefreshChildren();
}

void UDreamFlipCardBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	Super::OnSlotRemoved(InSlot);
	RefreshChildren();
}

void UDreamFlipCardBox::HandleSlateHovered()
{
	OnCardHovered.Broadcast();
}

void UDreamFlipCardBox::HandleSlateUnhovered()
{
	OnCardUnhovered.Broadcast();
}

void UDreamFlipCardBox::PushAllPropertiesToSlate()
{
	if (!MyFlipCardBox.IsValid())
	{
		return;
	}

	MyFlipCardBox->SetFlipAngle(FlipAngle);
	MyFlipCardBox->SetHorizontalSegments(HorizontalSegments);
	MyFlipCardBox->SetVerticalSegments(VerticalSegments);
	MyFlipCardBox->SetPerspectiveStrength(PerspectiveStrength);
	MyFlipCardBox->SetDepthOffset(DepthOffset);
	MyFlipCardBox->SetTransformPivot(TransformPivot);
	MyFlipCardBox->SetRetainedRenderScale(RetainedRenderScale);
	MyFlipCardBox->SetRenderEveryFrame(bRenderEveryFrame);
	MyFlipCardBox->SetMirrorBackFace(bMirrorBackFace);
}

void UDreamFlipCardBox::RefreshChildren()
{
	if (!MyFlipCardBox.IsValid())
	{
		return;
	}

	TSharedPtr<SWidget> FrontContent;
	if (UWidget* FrontWidget = GetFrontWidget())
	{
		FrontContent = FrontWidget->TakeWidget();
	}

	TSharedPtr<SWidget> BackContent;
	if (UWidget* BackWidget = GetBackWidget())
	{
		BackContent = BackWidget->TakeWidget();
	}

	MyFlipCardBox->SetFrontContent(FrontContent);
	MyFlipCardBox->SetBackContent(BackContent);
}

#undef LOCTEXT_NAMESPACE
