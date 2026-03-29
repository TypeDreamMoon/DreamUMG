// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/SDreamPseudo3DBox.h"

#include "Layout/Clipping.h"
#include "Layout/Geometry.h"
#include "Math/Quat.h"
#include "Types/PaintArgs.h"
#include "Widgets/WidgetPixelSnapping.h"
#include "Widgets/SNullWidget.h"

void SDreamPseudo3DBox::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(InArgs._Content.Widget);
	SetPixelSnapping(EWidgetPixelSnapping::Disabled);
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
	const int32 ClampedSegments = FMath::Clamp(InPerspectiveSegments, 1, 32);
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
	const float NearClipDistance = FMath::Max(1.0f, CameraDistance * 0.05f);
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;

	const int32 SegmentCountX = FMath::Clamp(PerspectiveSegments, 1, 32);
	const int32 SegmentCountY = FMath::Clamp(PerspectiveSegments, 1, 32);

	struct FSegmentPaintData
	{
		float StartX = 0.0f;
		float EndX = 0.0f;
		float StartY = 0.0f;
		float EndY = 0.0f;
		float AverageDepth = 0.0f;
		FVector2f TopLeft = FVector2f::ZeroVector;
		FVector2f TopRight = FVector2f::ZeroVector;
		FVector2f BottomLeft = FVector2f::ZeroVector;
		FVector2f BottomRight = FVector2f::ZeroVector;
	};

	TArray<FSegmentPaintData, TInlineAllocator<128>> SegmentsToPaint;
	SegmentsToPaint.Reserve(SegmentCountX * SegmentCountY);

	for (int32 SegmentY = 0; SegmentY < SegmentCountY; ++SegmentY)
	{
		const float SegmentStartY = LocalSize.Y * static_cast<float>(SegmentY) / static_cast<float>(SegmentCountY);
		const float SegmentEndY = LocalSize.Y * static_cast<float>(SegmentY + 1) / static_cast<float>(SegmentCountY);

		for (int32 SegmentX = 0; SegmentX < SegmentCountX; ++SegmentX)
		{
			const float SegmentStartX = LocalSize.X * static_cast<float>(SegmentX) / static_cast<float>(SegmentCountX);
			const float SegmentEndX = LocalSize.X * static_cast<float>(SegmentX + 1) / static_cast<float>(SegmentCountX);
			const FVector TopLeft3D = RotationQuat.RotateVector(BuildLocalPoint(FVector2f(SegmentStartX, SegmentStartY), LocalSize));
			const FVector TopRight3D = RotationQuat.RotateVector(BuildLocalPoint(FVector2f(SegmentEndX, SegmentStartY), LocalSize));
			const FVector BottomLeft3D = RotationQuat.RotateVector(BuildLocalPoint(FVector2f(SegmentStartX, SegmentEndY), LocalSize));
			const FVector BottomRight3D = RotationQuat.RotateVector(BuildLocalPoint(FVector2f(SegmentEndX, SegmentEndY), LocalSize));

			const FVector TangentX = ((TopRight3D - TopLeft3D) + (BottomRight3D - BottomLeft3D)) * 0.5f;
			const FVector TangentY = ((BottomLeft3D - TopLeft3D) + (BottomRight3D - TopRight3D)) * 0.5f;
			const FVector SurfaceNormal = FVector::CrossProduct(TangentX, TangentY);

			if (SurfaceNormal.Z <= UE_KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const float TopLeftDistance = CameraDistance - TopLeft3D.Z;
			const float TopRightDistance = CameraDistance - TopRight3D.Z;
			const float BottomLeftDistance = CameraDistance - BottomLeft3D.Z;
			const float BottomRightDistance = CameraDistance - BottomRight3D.Z;
			const float MinDistance = FMath::Min(
				FMath::Min(TopLeftDistance, TopRightDistance),
				FMath::Min(BottomLeftDistance, BottomRightDistance));

			// Once a segment reaches the near plane, affine reprojection becomes unstable and causes spikes.
			if (MinDistance <= NearClipDistance)
			{
				continue;
			}

			FSegmentPaintData& SegmentData = SegmentsToPaint.AddDefaulted_GetRef();
			SegmentData.StartX = SegmentStartX;
			SegmentData.EndX = SegmentEndX;
			SegmentData.StartY = SegmentStartY;
			SegmentData.EndY = SegmentEndY;
			SegmentData.AverageDepth = 0.25f * (TopLeft3D.Z + TopRight3D.Z + BottomLeft3D.Z + BottomRight3D.Z);
			SegmentData.TopLeft = ProjectRotatedPoint(TopLeft3D, PivotPixels, CameraDistance, NearClipDistance);
			SegmentData.TopRight = ProjectRotatedPoint(TopRight3D, PivotPixels, CameraDistance, NearClipDistance);
			SegmentData.BottomLeft = ProjectRotatedPoint(BottomLeft3D, PivotPixels, CameraDistance, NearClipDistance);
			SegmentData.BottomRight = ProjectRotatedPoint(BottomRight3D, PivotPixels, CameraDistance, NearClipDistance);
		}
	}

	SegmentsToPaint.Sort([](const FSegmentPaintData& A, const FSegmentPaintData& B)
	{
		return A.AverageDepth < B.AverageDepth;
	});

	int32 MaxLayer = LayerId;
	for (const FSegmentPaintData& SegmentData : SegmentsToPaint)
	{
		const float SegmentWidth = FMath::Max(SegmentData.EndX - SegmentData.StartX, 1.0f);
		const float SegmentHeight = FMath::Max(SegmentData.EndY - SegmentData.StartY, 1.0f);
		const float CenterX = 0.5f * (SegmentData.StartX + SegmentData.EndX);
		const float CenterY = 0.5f * (SegmentData.StartY + SegmentData.EndY);

		const FVector2f BasisX = ((SegmentData.TopRight - SegmentData.TopLeft) + (SegmentData.BottomRight - SegmentData.BottomLeft)) * (0.5f / SegmentWidth);
		const FVector2f BasisY = ((SegmentData.BottomLeft - SegmentData.TopLeft) + (SegmentData.BottomRight - SegmentData.TopRight)) * (0.5f / SegmentHeight);
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
		const FSlateRect SegmentCullingRect = bIgnoreClipping
			? SegmentClip.GetBoundingBox()
			: MyCullingRect.IntersectionWith(SegmentClip.GetBoundingBox());
		MaxLayer = FMath::Max(MaxLayer, ChildWidget->Paint(Args.WithNewParent(this), SegmentGeometry, SegmentCullingRect, OutDrawElements, LayerId + 1, CompoundedWidgetStyle, bShouldBeEnabled));
		OutDrawElements.PopClip();
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

FVector SDreamPseudo3DBox::BuildLocalPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize) const
{
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;
	return FVector(
		LocalPoint.X - PivotPixels.X,
		LocalPoint.Y - PivotPixels.Y,
		DepthOffset);
}

FVector2f SDreamPseudo3DBox::ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance, float NearClipDistance) const
{
	const float SafeDistance = FMath::Max(NearClipDistance, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}
