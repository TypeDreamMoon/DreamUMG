// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/SDreamCurvedBox.h"

#include "Layout/Clipping.h"
#include "Layout/Geometry.h"
#include "Math/Quat.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderTransform.h"
#include "Types/PaintArgs.h"
#include "Widgets/WidgetPixelSnapping.h"
#include "Widgets/SNullWidget.h"

void SDreamCurvedBox::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(InArgs._Content.Widget);
	SetPixelSnapping(EWidgetPixelSnapping::Disabled);
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
	const int32 ClampedSegments = FMath::Clamp(InCurveSegments, 2, 128);
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

	const int32 SegmentCount = FMath::Clamp(CurveSegments, 2, 128);
	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = LargestDimension * FMath::Lerp(8.0f, 1.25f, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;

	struct FSegmentPaintData
	{
		float StartX = 0.0f;
		float EndX = 0.0f;
		float AverageDepth = 0.0f;
		FVector2f TopLeft = FVector2f::ZeroVector;
		FVector2f TopRight = FVector2f::ZeroVector;
		FVector2f BottomLeft = FVector2f::ZeroVector;
		FVector2f BottomRight = FVector2f::ZeroVector;
	};

	TArray<FSegmentPaintData, TInlineAllocator<64>> SegmentsToPaint;
	SegmentsToPaint.Reserve(SegmentCount);

	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const float SegmentStartX = LocalSize.X * static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount);
		const float SegmentEndX = LocalSize.X * static_cast<float>(SegmentIndex + 1) / static_cast<float>(SegmentCount);
		const FVector TopLeft3D = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(SegmentStartX, 0.0f), LocalSize));
		const FVector TopRight3D = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(SegmentEndX, 0.0f), LocalSize));
		const FVector BottomLeft3D = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(SegmentStartX, LocalSize.Y), LocalSize));
		const FVector BottomRight3D = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(SegmentEndX, LocalSize.Y), LocalSize));
		const FVector TangentX = ((TopRight3D - TopLeft3D) + (BottomRight3D - BottomLeft3D)) * 0.5f;
		const FVector TangentY = ((BottomLeft3D - TopLeft3D) + (BottomRight3D - TopRight3D)) * 0.5f;
		const FVector SurfaceNormal = FVector::CrossProduct(TangentX, TangentY);

		if (SurfaceNormal.Z <= UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}

		FSegmentPaintData& SegmentData = SegmentsToPaint.AddDefaulted_GetRef();
		SegmentData.StartX = SegmentStartX;
		SegmentData.EndX = SegmentEndX;
		SegmentData.AverageDepth = 0.25f * (TopLeft3D.Z + TopRight3D.Z + BottomLeft3D.Z + BottomRight3D.Z);
		SegmentData.TopLeft = ProjectRotatedPoint(TopLeft3D, PivotPixels, CameraDistance);
		SegmentData.TopRight = ProjectRotatedPoint(TopRight3D, PivotPixels, CameraDistance);
		SegmentData.BottomLeft = ProjectRotatedPoint(BottomLeft3D, PivotPixels, CameraDistance);
		SegmentData.BottomRight = ProjectRotatedPoint(BottomRight3D, PivotPixels, CameraDistance);
	}

	SegmentsToPaint.Sort([](const FSegmentPaintData& A, const FSegmentPaintData& B)
	{
		return A.AverageDepth < B.AverageDepth;
	});

	int32 MaxLayer = LayerId;
	for (const FSegmentPaintData& SegmentData : SegmentsToPaint)
	{
		const float SegmentWidth = FMath::Max(SegmentData.EndX - SegmentData.StartX, 1.0f);
		const float CenterX = 0.5f * (SegmentData.StartX + SegmentData.EndX);
		const float CenterY = 0.5f * LocalSize.Y;
		const FVector2f BasisX = ((SegmentData.TopRight - SegmentData.TopLeft) + (SegmentData.BottomRight - SegmentData.BottomLeft)) * (0.5f / SegmentWidth);
		const FVector2f BasisY = ((SegmentData.BottomLeft - SegmentData.TopLeft) + (SegmentData.BottomRight - SegmentData.TopRight)) * (0.5f / LocalSize.Y);
		const FMatrix2x2 RenderMatrix(BasisX.X, BasisX.Y, BasisY.X, BasisY.Y);
		const FVector2f CenterProjected = (SegmentData.TopLeft + SegmentData.TopRight + SegmentData.BottomLeft + SegmentData.BottomRight) * 0.25f;
		const FVector2f Translation = CenterProjected - TransformPoint(RenderMatrix, FVector2f(CenterX, CenterY));
		const FSlateRenderTransform SegmentTransform(Concatenate(RenderMatrix, Translation));
		const FGeometry SegmentGeometry = AllottedGeometry.MakeChild(LocalSize, FSlateLayoutTransform(), SegmentTransform, FVector2f::ZeroVector);

		const FSlateClippingZone SegmentClip(
			AllottedGeometry.LocalToAbsolute(SegmentData.TopLeft),
			AllottedGeometry.LocalToAbsolute(SegmentData.TopRight),
			AllottedGeometry.LocalToAbsolute(SegmentData.BottomLeft),
			AllottedGeometry.LocalToAbsolute(SegmentData.BottomRight));

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

FVector SDreamCurvedBox::BuildCurvedPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize) const
{
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;
	const float RelativeX = LocalPoint.X - PivotPixels.X;
	const float RelativeY = LocalPoint.Y - PivotPixels.Y;
	const float TotalAngleRadians = FMath::DegreesToRadians(CurveAngle);
	const float AbsAngleRadians = FMath::Abs(TotalAngleRadians);

	FVector CurvedPoint(RelativeX, RelativeY, DepthOffset);
	if (!FMath::IsNearlyZero(AbsAngleRadians))
	{
		const float Radius = LocalSize.X / AbsAngleRadians;
		const float PointAngle = RelativeX * (AbsAngleRadians / LocalSize.X);
		const float CurveDepthSign = CurveAngle >= 0.0f ? -1.0f : 1.0f;

		CurvedPoint.X = Radius * FMath::Sin(PointAngle);
		CurvedPoint.Y = RelativeY;
		CurvedPoint.Z = DepthOffset + CurveDepthSign * Radius * (1.0f - FMath::Cos(PointAngle));
	}

	return CurvedPoint;
}

FVector2f SDreamCurvedBox::ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance) const
{
	const float SafeDistance = FMath::Max(1.0f, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}
