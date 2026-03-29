// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SDreamCurvedBox.h"

#include "Layout/Clipping.h"
#include "Layout/Geometry.h"
#include "Math/Quat.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderTransform.h"
#include "Types/PaintArgs.h"
#include "Widgets/SNullWidget.h"

void SDreamCurvedBox::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(InArgs._Content.Widget);
}

void SDreamCurvedBox::SetContent(const TSharedRef<SWidget>& InContent)
{
	ChildSlot.AttachWidget(InContent);
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SDreamCurvedBox::SetRotation(FRotator InRotation)
{
	if (!Rotation.Equals(InRotation))
	{
		Rotation = InRotation;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamCurvedBox::SetCurveAngle(float InCurveAngle)
{
	if (!FMath::IsNearlyEqual(CurveAngle, InCurveAngle))
	{
		CurveAngle = InCurveAngle;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamCurvedBox::SetCurveSegments(int32 InCurveSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InCurveSegments, 2, 64);
	if (CurveSegments != ClampedSegments)
	{
		CurveSegments = ClampedSegments;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamCurvedBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	const float ClampedStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (!FMath::IsNearlyEqual(PerspectiveStrength, ClampedStrength))
	{
		PerspectiveStrength = ClampedStrength;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamCurvedBox::SetDepthOffset(float InDepthOffset)
{
	if (!FMath::IsNearlyEqual(DepthOffset, InDepthOffset))
	{
		DepthOffset = InDepthOffset;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamCurvedBox::SetTransformPivot(FVector2D InTransformPivot)
{
	const FVector2D ClampedPivot(FMath::Clamp(InTransformPivot.X, 0.0, 1.0), FMath::Clamp(InTransformPivot.Y, 0.0, 1.0));
	if (!TransformPivot.Equals(ClampedPivot))
	{
		TransformPivot = ClampedPivot;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

int32 SDreamCurvedBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
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

	const int32 SegmentCount = FMath::Clamp(CurveSegments, 2, 64);
	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = LargestDimension * FMath::Lerp(8.0f, 1.25f, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));

	int32 MaxLayer = LayerId;
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const float SegmentStartX = LocalSize.X * static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount);
		const float SegmentEndX = LocalSize.X * static_cast<float>(SegmentIndex + 1) / static_cast<float>(SegmentCount);
		const float SegmentWidth = FMath::Max(SegmentEndX - SegmentStartX, 1.0f);
		const float CenterX = 0.5f * (SegmentStartX + SegmentEndX);
		const float CenterY = 0.5f * LocalSize.Y;

		const FVector2f TopLeft = ProjectLocalPoint(FVector2f(SegmentStartX, 0.0f), LocalSize, CameraDistance, RotationQuat);
		const FVector2f TopRight = ProjectLocalPoint(FVector2f(SegmentEndX, 0.0f), LocalSize, CameraDistance, RotationQuat);
		const FVector2f BottomLeft = ProjectLocalPoint(FVector2f(SegmentStartX, LocalSize.Y), LocalSize, CameraDistance, RotationQuat);
		const FVector2f BottomRight = ProjectLocalPoint(FVector2f(SegmentEndX, LocalSize.Y), LocalSize, CameraDistance, RotationQuat);

		const FVector2f BasisX = ((TopRight - TopLeft) + (BottomRight - BottomLeft)) * (0.5f / SegmentWidth);
		const FVector2f BasisY = ((BottomLeft - TopLeft) + (BottomRight - TopRight)) * (0.5f / LocalSize.Y);
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
		const FSlateRect SegmentCullingRect = MyCullingRect.IntersectionWith(SegmentClip.GetBoundingBox());
		MaxLayer = FMath::Max(MaxLayer, ChildWidget->Paint(Args.WithNewParent(this), SegmentGeometry, SegmentCullingRect, OutDrawElements, LayerId + 1, CompoundedWidgetStyle, bShouldBeEnabled));
		OutDrawElements.PopClip();
	}

	return MaxLayer;
}

FVector2D SDreamCurvedBox::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return ChildSlot.GetWidget()->GetDesiredSize();
}

void SDreamCurvedBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
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

FVector2f SDreamCurvedBox::ProjectLocalPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize, float CameraDistance, const FQuat& RotationQuat) const
{
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;
	const float RelativeX = LocalPoint.X - PivotPixels.X;
	const float RelativeY = LocalPoint.Y - PivotPixels.Y;
	const float TotalAngleRadians = FMath::DegreesToRadians(CurveAngle);

	FVector CurvedPoint(RelativeX, RelativeY, DepthOffset);
	if (!FMath::IsNearlyZero(TotalAngleRadians))
	{
		const float Radius = LocalSize.X / FMath::Abs(TotalAngleRadians);
		const float PointAngle = RelativeX * (TotalAngleRadians / LocalSize.X);
		const float CurveDepthSign = CurveAngle >= 0.0f ? -1.0f : 1.0f;

		CurvedPoint.X = Radius * FMath::Sin(PointAngle);
		CurvedPoint.Y = RelativeY;
		CurvedPoint.Z = DepthOffset + CurveDepthSign * Radius * (1.0f - FMath::Cos(PointAngle));
	}

	const FVector RotatedPoint = RotationQuat.RotateVector(CurvedPoint);
	const float SafeDistance = FMath::Max(1.0f, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}
