// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/SDreamMeshRetainerBox.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/HittestGrid.h"
#include "Layout/Geometry.h"
#include "Layout/WidgetPath.h"
#include "Math/Quat.h"
#include "Rendering/DrawElements.h"
#include "Slate/WidgetRenderer.h"
#include "Types/PaintArgs.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SVirtualWindow.h"
#include "Widgets/WidgetPixelSnapping.h"

SDreamMeshRetainerBox::SDreamMeshRetainerBox()
{
	SetPixelSnapping(EWidgetPixelSnapping::Disabled);
}

SDreamMeshRetainerBox::~SDreamMeshRetainerBox()
{
	ReleaseRenderResources();
}

void SDreamMeshRetainerBox::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(InArgs._Content.Widget);
	SetCanTick(false);
}

void SDreamMeshRetainerBox::SetContent(const TSharedRef<SWidget>& InContent)
{
	ChildSlot.AttachWidget(InContent);
	RequestRender();
	Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SDreamMeshRetainerBox::SetRotation(FRotator InRotation)
{
	if (!Rotation.Equals(InRotation))
	{
		Rotation = InRotation;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetCurveAngle(float InCurveAngle)
{
	if (!FMath::IsNearlyEqual(CurveAngle, InCurveAngle))
	{
		CurveAngle = InCurveAngle;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetCurveSegments(int32 InCurveSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InCurveSegments, 2, 256);
	if (CurveSegments != ClampedSegments)
	{
		CurveSegments = ClampedSegments;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetVerticalSegments(int32 InVerticalSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InVerticalSegments, 1, 64);
	if (VerticalSegments != ClampedSegments)
	{
		VerticalSegments = ClampedSegments;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	const float ClampedStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (!FMath::IsNearlyEqual(PerspectiveStrength, ClampedStrength))
	{
		PerspectiveStrength = ClampedStrength;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetDepthOffset(float InDepthOffset)
{
	if (!FMath::IsNearlyEqual(DepthOffset, InDepthOffset))
	{
		DepthOffset = InDepthOffset;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetTransformPivot(FVector2D InTransformPivot)
{
	const FVector2D ClampedPivot(FMath::Clamp(InTransformPivot.X, 0.0, 1.0), FMath::Clamp(InTransformPivot.Y, 0.0, 1.0));
	if (!TransformPivot.Equals(ClampedPivot))
	{
		TransformPivot = ClampedPivot;
		InvalidateMeshRetainer();
	}
}

void SDreamMeshRetainerBox::SetRetainedRenderScale(float InRetainedRenderScale)
{
	const float ClampedScale = FMath::Max(0.1f, InRetainedRenderScale);
	if (!FMath::IsNearlyEqual(RetainedRenderScale, ClampedScale))
	{
		RetainedRenderScale = ClampedScale;
		RequestRender();
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamMeshRetainerBox::SetRenderEveryFrame(bool bInRenderEveryFrame)
{
	if (bRenderEveryFrame != bInRenderEveryFrame)
	{
		bRenderEveryFrame = bInRenderEveryFrame;
		RequestRender();
	}
}

void SDreamMeshRetainerBox::RequestRender()
{
	bRenderRequested = true;
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 SDreamMeshRetainerBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2f LocalSize = AllottedGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return LayerId;
	}

	UpdateRenderTarget(LocalSize, InWidgetStyle, bParentEnabled);
	if (!RenderTarget.IsValid())
	{
		return LayerId;
	}

	SurfaceBrush.SetResourceObject(RenderTarget.Get());
	SurfaceBrush.ImageSize = FVector2D(RenderTargetSize);
	const FSlateResourceHandle ResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(SurfaceBrush);
	if (!ResourceHandle.IsValid())
	{
		return LayerId;
	}

	TArray<FSlateVertex> Vertices;
	TArray<SlateIndex> Indices;
	BuildSlateMesh(AllottedGeometry, LocalSize, Vertices, Indices);
	if (Vertices.IsEmpty() || Indices.IsEmpty())
	{
		return LayerId;
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, ResourceHandle, Vertices, Indices, nullptr, 0, 0);
	return LayerId;
}

FVector2D SDreamMeshRetainerBox::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return ChildSlot.GetWidget()->GetDesiredSize();
}

void SDreamMeshRetainerBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
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

FReply SDreamMeshRetainerBox::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RoutePointerDownEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

FReply SDreamMeshRetainerBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RoutePointerUpEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

FReply SDreamMeshRetainerBox::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		const bool bHandled = FSlateApplication::Get().RoutePointerMoveEvent(WidgetPath, RoutedEvent, false);
		bHasVirtualHover = true;
		return bHandled ? FReply::Handled() : FReply::Unhandled();
	}

	ClearVirtualHover(MouseEvent);
	return FReply::Unhandled();
}

FReply SDreamMeshRetainerBox::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RoutePointerDoubleClickEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

FReply SDreamMeshRetainerBox::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RouteMouseWheelOrGestureEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

void SDreamMeshRetainerBox::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	ClearVirtualHover(MouseEvent);
	SCompoundWidget::OnMouseLeave(MouseEvent);
}

FVector SDreamMeshRetainerBox::BuildCurvedPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize) const
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

FVector2f SDreamMeshRetainerBox::ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance) const
{
	const float SafeDistance = FMath::Max(1.0f, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}

void SDreamMeshRetainerBox::BuildSlateMesh(const FGeometry& AllottedGeometry, const FVector2f& LocalSize, TArray<FSlateVertex>& OutVertices, TArray<SlateIndex>& OutIndices) const
{
	const int32 SegmentCountX = FMath::Clamp(CurveSegments, 2, 256);
	const int32 SegmentCountY = FMath::Clamp(VerticalSegments, 1, 64);
	const int32 VertexCountX = SegmentCountX + 1;
	const int32 VertexCountY = SegmentCountY + 1;
	const int32 VertexCount = VertexCountX * VertexCountY;
	if (static_cast<uint32>(VertexCount) > static_cast<uint32>(TNumericLimits<SlateIndex>::Max()))
	{
		return;
	}

	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = LargestDimension * FMath::Lerp(8.0f, 1.25f, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;
	const FSlateRenderTransform& RenderTransform = AllottedGeometry.GetAccumulatedRenderTransform();
	const FColor VertexColor = FColor::White;

	OutVertices.Reserve(VertexCount);
	for (int32 YIndex = 0; YIndex < VertexCountY; ++YIndex)
	{
		const float V = static_cast<float>(YIndex) / static_cast<float>(SegmentCountY);
		const float LocalY = LocalSize.Y * V;
		for (int32 XIndex = 0; XIndex < VertexCountX; ++XIndex)
		{
			const float U = static_cast<float>(XIndex) / static_cast<float>(SegmentCountX);
			const float LocalX = LocalSize.X * U;
			const FVector RotatedPoint = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(LocalX, LocalY), LocalSize));
			const FVector2f ProjectedPoint = ProjectRotatedPoint(RotatedPoint, PivotPixels, CameraDistance);

			OutVertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
				RenderTransform,
				ProjectedPoint,
				FVector2f(U, V),
				VertexColor));
		}
	}

	OutIndices.Reserve(SegmentCountX * SegmentCountY * 6);
	for (int32 YIndex = 0; YIndex < SegmentCountY; ++YIndex)
	{
		for (int32 XIndex = 0; XIndex < SegmentCountX; ++XIndex)
		{
			const SlateIndex TopLeft = static_cast<SlateIndex>(YIndex * VertexCountX + XIndex);
			const SlateIndex TopRight = static_cast<SlateIndex>(TopLeft + 1);
			const SlateIndex BottomLeft = static_cast<SlateIndex>((YIndex + 1) * VertexCountX + XIndex);
			const SlateIndex BottomRight = static_cast<SlateIndex>(BottomLeft + 1);

			OutIndices.Add(TopLeft);
			OutIndices.Add(TopRight);
			OutIndices.Add(BottomLeft);
			OutIndices.Add(TopRight);
			OutIndices.Add(BottomRight);
			OutIndices.Add(BottomLeft);
		}
	}
}

void SDreamMeshRetainerBox::UpdateRenderTarget(const FVector2f& LocalSize, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FIntPoint DesiredRenderTargetSize(
		FMath::Max(1, FMath::RoundToInt(LocalSize.X * RetainedRenderScale)),
		FMath::Max(1, FMath::RoundToInt(LocalSize.Y * RetainedRenderScale)));

	if (!RenderTarget.IsValid() || RenderTargetSize != DesiredRenderTargetSize)
	{
		RenderTarget.Reset(FWidgetRenderer::CreateTargetFor(FVector2D(DesiredRenderTargetSize), TF_Bilinear, false));
		RenderTargetSize = DesiredRenderTargetSize;
		SurfaceBrush.SetResourceObject(RenderTarget.Get());
		SurfaceBrush.DrawAs = ESlateBrushDrawType::Image;
		SurfaceBrush.Tiling = ESlateBrushTileType::NoTile;
		SurfaceBrush.ImageSize = FVector2D(DesiredRenderTargetSize);
		bRenderRequested = true;
	}

	if (!RenderTarget.IsValid())
	{
		return;
	}

	if (!WidgetRenderer.IsValid())
	{
		WidgetRenderer = MakeUnique<FWidgetRenderer>(false, true);
	}

	if (bRenderEveryFrame || bRenderRequested)
	{
		const TSharedRef<SWidget>& ChildWidget = ChildSlot.GetWidget();
		if (ChildWidget->GetVisibility().IsVisible())
		{
			const float DeltaTime = 0.0f;
			if (!VirtualWindow.IsValid())
			{
				SAssignNew(VirtualWindow, SVirtualWindow).Size(FVector2D(DesiredRenderTargetSize));
			}
			if (!HitTestGrid.IsValid())
			{
				HitTestGrid = MakeShared<FHittestGrid>();
			}

			const TSharedPtr<SWidget> OldParent = ChildWidget->GetParentWidget();
			VirtualWindow->SetContent(ChildWidget);
			VirtualWindow->Resize(FVector2D(DesiredRenderTargetSize));
			WidgetRenderer->DrawWindow(RenderTarget.Get(), *HitTestGrid, VirtualWindow.ToSharedRef(), 1.0f, FVector2D(DesiredRenderTargetSize), DeltaTime);
			if (OldParent.IsValid())
			{
				ChildWidget->AssignParentWidget(OldParent);
			}
		}
		bRenderRequested = false;
	}
}

bool SDreamMeshRetainerBox::MapScreenPositionToVirtualWindow(const FGeometry& MyGeometry, const FVector2D& ScreenPosition, FVector2D& OutVirtualPosition) const
{
	if (RenderTargetSize.X <= 0 || RenderTargetSize.Y <= 0)
	{
		return false;
	}

	const FVector2f LocalSize = MyGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return false;
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(ScreenPosition);
	const int32 SegmentCountX = FMath::Clamp(CurveSegments, 2, 256);
	const int32 SegmentCountY = FMath::Clamp(VerticalSegments, 1, 64);
	const int32 VertexCountX = SegmentCountX + 1;
	const int32 VertexCountY = SegmentCountY + 1;
	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = LargestDimension * FMath::Lerp(8.0f, 1.25f, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const FQuat RotationQuat =
		FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(Rotation.Roll)) *
		FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(Rotation.Yaw)) *
		FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(Rotation.Pitch));
	const FVector2f PivotPixels = FVector2f(TransformPivot) * LocalSize;

	struct FHitVertex
	{
		FVector2D Position;
		FVector2D UV;
	};

	TArray<FHitVertex, TInlineAllocator<512>> HitVertices;
	HitVertices.Reserve(VertexCountX * VertexCountY);
	for (int32 YIndex = 0; YIndex < VertexCountY; ++YIndex)
	{
		const float V = static_cast<float>(YIndex) / static_cast<float>(SegmentCountY);
		const float LocalY = LocalSize.Y * V;
		for (int32 XIndex = 0; XIndex < VertexCountX; ++XIndex)
		{
			const float U = static_cast<float>(XIndex) / static_cast<float>(SegmentCountX);
			const float LocalX = LocalSize.X * U;
			const FVector RotatedPoint = RotationQuat.RotateVector(BuildCurvedPoint(FVector2f(LocalX, LocalY), LocalSize));
			const FVector2f ProjectedPoint = ProjectRotatedPoint(RotatedPoint, PivotPixels, CameraDistance);
			HitVertices.Add({FVector2D(ProjectedPoint), FVector2D(U, V)});
		}
	}

	auto TryTriangle = [&LocalPosition, &OutVirtualPosition, this](const FHitVertex& A, const FHitVertex& B, const FHitVertex& C) -> bool
	{
		const FVector2D V0 = B.Position - A.Position;
		const FVector2D V1 = C.Position - A.Position;
		const FVector2D V2 = LocalPosition - A.Position;
		const double Denominator = V0.X * V1.Y - V1.X * V0.Y;
		if (FMath::IsNearlyZero(Denominator))
		{
			return false;
		}

		const double InvDenominator = 1.0 / Denominator;
		const double UWeight = (V2.X * V1.Y - V1.X * V2.Y) * InvDenominator;
		const double VWeight = (V0.X * V2.Y - V2.X * V0.Y) * InvDenominator;
		const double WWeight = 1.0 - UWeight - VWeight;
		constexpr double EdgeTolerance = -0.0001;
		if (UWeight < EdgeTolerance || VWeight < EdgeTolerance || WWeight < EdgeTolerance)
		{
			return false;
		}

		const FVector2D UV = A.UV * WWeight + B.UV * UWeight + C.UV * VWeight;
		OutVirtualPosition = FVector2D(UV.X * RenderTargetSize.X, UV.Y * RenderTargetSize.Y);
		return true;
	};

	for (int32 YIndex = 0; YIndex < SegmentCountY; ++YIndex)
	{
		for (int32 XIndex = 0; XIndex < SegmentCountX; ++XIndex)
		{
			const int32 TopLeft = YIndex * VertexCountX + XIndex;
			const int32 TopRight = TopLeft + 1;
			const int32 BottomLeft = (YIndex + 1) * VertexCountX + XIndex;
			const int32 BottomRight = BottomLeft + 1;

			if (TryTriangle(HitVertices[TopLeft], HitVertices[TopRight], HitVertices[BottomLeft]) ||
				TryTriangle(HitVertices[TopRight], HitVertices[BottomRight], HitVertices[BottomLeft]))
			{
				return true;
			}
		}
	}

	return false;
}

bool SDreamMeshRetainerBox::BuildRoutedPointerEvent(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FPointerEvent& OutPointerEvent, FWidgetPath& OutWidgetPath) const
{
	if (!HitTestGrid.IsValid() || !VirtualWindow.IsValid())
	{
		return false;
	}

	FVector2D VirtualPosition;
	if (!MapScreenPositionToVirtualWindow(MyGeometry, MouseEvent.GetScreenSpacePosition(), VirtualPosition))
	{
		return false;
	}

	FVector2D LastVirtualPosition;
	if (!MapScreenPositionToVirtualWindow(MyGeometry, MouseEvent.GetLastScreenSpacePosition(), LastVirtualPosition))
	{
		LastVirtualPosition = VirtualPosition;
	}

	TArray<FWidgetAndPointer> BubblePath = HitTestGrid->GetBubblePath(VirtualPosition, 0.0f, false, MouseEvent.GetUserIndex());
	if (BubblePath.IsEmpty())
	{
		return false;
	}

	OutWidgetPath = FWidgetPath(MakeArrayView(BubblePath));
	if (!OutWidgetPath.IsValid())
	{
		return false;
	}

	OutPointerEvent = FPointerEvent(MouseEvent, VirtualPosition, LastVirtualPosition);
	return true;
}

void SDreamMeshRetainerBox::ClearVirtualHover(const FPointerEvent& MouseEvent) const
{
	if (!bHasVirtualHover)
	{
		return;
	}

	FSlateApplication::Get().RoutePointerMoveEvent(FWidgetPath(), MouseEvent, false);
	bHasVirtualHover = false;
}

void SDreamMeshRetainerBox::ReleaseRenderResources()
{
	bHasVirtualHover = false;
	WidgetRenderer.Reset();
	VirtualWindow.Reset();
	HitTestGrid.Reset();
	RenderTarget.Reset();
	RenderTargetSize = FIntPoint::ZeroValue;
}

void SDreamMeshRetainerBox::InvalidateMeshRetainer()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
