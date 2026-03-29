// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SDreamPseudo3DBox.h"

#include "Layout/Clipping.h"
#include "Layout/Geometry.h"
#include "Math/Quat.h"
#include "Types/PaintArgs.h"
#include "Widgets/SNullWidget.h"

void SDreamPseudo3DBox::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(InArgs._Content.Widget);
}

void SDreamPseudo3DBox::SetContent(const TSharedRef<SWidget>& InContent)
{
	ChildSlot.AttachWidget(InContent);
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SDreamPseudo3DBox::SetRotation(FRotator InRotation)
{
	if (!Rotation.Equals(InRotation))
	{
		Rotation = InRotation;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	const float ClampedStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (!FMath::IsNearlyEqual(PerspectiveStrength, ClampedStrength))
	{
		PerspectiveStrength = ClampedStrength;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetFieldOfView(float InFieldOfView)
{
	const float ClampedFieldOfView = FMath::Clamp(InFieldOfView, 1.0f, 170.0f);
	if (!FMath::IsNearlyEqual(FieldOfView, ClampedFieldOfView))
	{
		FieldOfView = ClampedFieldOfView;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetDepthOffset(float InDepthOffset)
{
	if (!FMath::IsNearlyEqual(DepthOffset, InDepthOffset))
	{
		DepthOffset = InDepthOffset;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetPerspectiveSegments(int32 InPerspectiveSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InPerspectiveSegments, 1, 12);
	if (PerspectiveSegments != ClampedSegments)
	{
		PerspectiveSegments = ClampedSegments;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetTransformPivot(FVector2D InTransformPivot)
{
	const FVector2D ClampedPivot(FMath::Clamp(InTransformPivot.X, 0.0, 1.0), FMath::Clamp(InTransformPivot.Y, 0.0, 1.0));
	if (!TransformPivot.Equals(ClampedPivot))
	{
		TransformPivot = ClampedPivot;
		InvalidatePseudo3DTransform();
	}
}

void SDreamPseudo3DBox::SetIgnoreClipping(bool bInIgnoreClipping)
{
	if (bIgnoreClipping != bInIgnoreClipping)
	{
		bIgnoreClipping = bInIgnoreClipping;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamPseudo3DBox::InvalidatePseudo3DTransform()
{
	Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::RenderTransform);
}

int32 SDreamPseudo3DBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const TSharedRef<SWidget>& ChildWidget = ChildSlot.GetWidget();
	if (!ChildWidget->GetVisibility().IsVisible())
	{
		return LayerId;
	}

	const FVector2f LocalSize = AllottedGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return LayerId;
	}

	const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);
	FWidgetStyle CompoundedWidgetStyle = FWidgetStyle(InWidgetStyle)
		.BlendColorAndOpacityTint(GetColorAndOpacity())
		.SetForegroundColor(bShouldBeEnabled ? GetForegroundColor() : GetDisabledForegroundColor());

	const float EffectiveFieldOfView = FMath::Lerp(1.0f, FieldOfView, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = 0.5f * LargestDimension / FMath::Tan(0.5f * FMath::DegreesToRadians(EffectiveFieldOfView));
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));

	const int32 SegmentCountX = FMath::Clamp(PerspectiveSegments, 1, 12);
	const int32 SegmentCountY = FMath::Clamp(PerspectiveSegments, 1, 12);
	int32 MaxLayer = LayerId;

	for (int32 SegmentY = 0; SegmentY < SegmentCountY; ++SegmentY)
	{
		const float SegmentStartY = LocalSize.Y * static_cast<float>(SegmentY) / static_cast<float>(SegmentCountY);
		const float SegmentEndY = LocalSize.Y * static_cast<float>(SegmentY + 1) / static_cast<float>(SegmentCountY);
		const float SegmentHeight = FMath::Max(SegmentEndY - SegmentStartY, 1.0f);
		const float CenterY = 0.5f * (SegmentStartY + SegmentEndY);

		for (int32 SegmentX = 0; SegmentX < SegmentCountX; ++SegmentX)
		{
			const float SegmentStartX = LocalSize.X * static_cast<float>(SegmentX) / static_cast<float>(SegmentCountX);
			const float SegmentEndX = LocalSize.X * static_cast<float>(SegmentX + 1) / static_cast<float>(SegmentCountX);
			const float SegmentWidth = FMath::Max(SegmentEndX - SegmentStartX, 1.0f);
			const float CenterX = 0.5f * (SegmentStartX + SegmentEndX);

			const FVector2f TopLeft = ProjectLocalPoint(FVector2f(SegmentStartX, SegmentStartY), LocalSize, CameraDistance, RotationQuat);
			const FVector2f TopRight = ProjectLocalPoint(FVector2f(SegmentEndX, SegmentStartY), LocalSize, CameraDistance, RotationQuat);
			const FVector2f BottomLeft = ProjectLocalPoint(FVector2f(SegmentStartX, SegmentEndY), LocalSize, CameraDistance, RotationQuat);
			const FVector2f BottomRight = ProjectLocalPoint(FVector2f(SegmentEndX, SegmentEndY), LocalSize, CameraDistance, RotationQuat);

			const FVector2f BasisX = ((TopRight - TopLeft) + (BottomRight - BottomLeft)) * (0.5f / SegmentWidth);
			const FVector2f BasisY = ((BottomLeft - TopLeft) + (BottomRight - TopRight)) * (0.5f / SegmentHeight);
			const FMatrix2x2 RenderMatrix(BasisX.X, BasisX.Y, BasisY.X, BasisY.Y);
			const FVector2f CenterProjected = (TopLeft + TopRight + BottomLeft + BottomRight) * 0.25f;
			const FVector2f Translation = CenterProjected - TransformPoint(RenderMatrix, FVector2f(CenterX, CenterY));
			const FSlateRenderTransform SegmentTransform(Concatenate(RenderMatrix, Translation));
			const FGeometry SegmentGeometry = AllottedGeometry.MakeChild(LocalSize, FSlateLayoutTransform(), SegmentTransform, FVector2f::ZeroVector);

			const FSlateClippingZone SegmentClip(
				AllottedGeometry.LocalToAbsolute(TopLeft),
				AllottedGeometry.LocalToAbsolute(TopRight),
				AllottedGeometry.LocalToAbsolute(BottomLeft),
				AllottedGeometry.LocalToAbsolute(BottomRight));

			OutDrawElements.PushClip(SegmentClip);
			const FSlateRect SegmentCullingRect = bIgnoreClipping
				? SegmentClip.GetBoundingBox()
				: MyCullingRect.IntersectionWith(SegmentClip.GetBoundingBox());
			MaxLayer = FMath::Max(MaxLayer, ChildWidget->Paint(Args.WithNewParent(this), SegmentGeometry, SegmentCullingRect, OutDrawElements, LayerId + 1, CompoundedWidgetStyle, bShouldBeEnabled));
			OutDrawElements.PopClip();
		}
	}

	return MaxLayer;
}

FVector2D SDreamPseudo3DBox::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return ChildSlot.GetWidget()->GetDesiredSize();
}

void SDreamPseudo3DBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const EVisibility MyVisibility = GetVisibility();
	if (!ArrangedChildren.Accepts(MyVisibility))
	{
		return;
	}

	const TSharedRef<SWidget>& ChildWidget = ChildSlot.GetWidget();
	const EVisibility ChildVisibility = ChildWidget->GetVisibility();
	if (!ArrangedChildren.Accepts(ChildVisibility))
	{
		return;
	}

	ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(ChildWidget, AllottedGeometry.GetLocalSize(), FSlateLayoutTransform()));
}

FVector2f SDreamPseudo3DBox::ProjectLocalPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize, float CameraDistance, const FQuat& RotationQuat) const
{
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;
	const FVector LocalPoint3D(
		LocalPoint.X - PivotPixels.X,
		LocalPoint.Y - PivotPixels.Y,
		DepthOffset);
	const FVector RotatedPoint = RotationQuat.RotateVector(LocalPoint3D);
	const float SafeDistance = FMath::Max(1.0f, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}
