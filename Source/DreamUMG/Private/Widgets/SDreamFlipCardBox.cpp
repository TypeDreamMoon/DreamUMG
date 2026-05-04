// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/SDreamFlipCardBox.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/HittestGrid.h"
#include "Layout/Geometry.h"
#include "Layout/WidgetPath.h"
#include "Rendering/DrawElements.h"
#include "Slate/WidgetRenderer.h"
#include "Types/PaintArgs.h"
#include "Slate/SObjectWidget.h"
#if WITH_ACCESSIBILITY
#include "Widgets/Accessibility/SlateAccessibleWidgets.h"
#endif
#include "Widgets/Input/SButton.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SVirtualWindow.h"
#include "Widgets/WidgetPixelSnapping.h"

namespace
{
struct FVirtualButtonTarget
{
	TSharedPtr<SButton> SlateButton;
	UButton* WidgetButton = nullptr;
};

FVirtualButtonTarget FindVirtualButtonInPath(const FWidgetPath& WidgetPath)
{
	TArray<UUserWidget*, TInlineAllocator<4>> OwnerUserWidgets;
	for (int32 WidgetIndex = 0; WidgetIndex < WidgetPath.Widgets.Num(); ++WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[WidgetIndex];
		if (ArrangedWidget.Widget->GetWidgetClass().GetWidgetType() == SObjectWidget::StaticWidgetClass().GetWidgetType())
		{
			if (UUserWidget* UserWidget = StaticCastSharedRef<SObjectWidget>(ArrangedWidget.Widget)->GetWidgetObject())
			{
				OwnerUserWidgets.Add(UserWidget);
			}
		}
	}

	for (int32 WidgetIndex = WidgetPath.Widgets.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		const TSharedRef<SWidget>& SlateWidget = WidgetPath.Widgets[WidgetIndex].Widget;
		FVirtualButtonTarget Target;
		if (SlateWidget->GetWidgetClass().GetWidgetType() == SButton::StaticWidgetClass().GetWidgetType())
		{
			Target.SlateButton = StaticCastSharedRef<SButton>(SlateWidget);
		}

		for (int32 OwnerIndex = OwnerUserWidgets.Num() - 1; OwnerIndex >= 0; --OwnerIndex)
		{
			if (UWidget* Widget = OwnerUserWidgets[OwnerIndex]->GetWidgetHandle(SlateWidget))
			{
				if (UButton* Button = Cast<UButton>(Widget))
				{
					Target.WidgetButton = Button;
					break;
				}
			}
		}

		if (Target.SlateButton.IsValid() || Target.WidgetButton)
		{
			return Target;
		}
	}

	return FVirtualButtonTarget();
}

TSharedPtr<SWidget> GetVirtualButtonWidget(const FVirtualButtonTarget& Target)
{
	if (Target.SlateButton.IsValid())
	{
		return StaticCastSharedPtr<SWidget>(Target.SlateButton);
	}

	return Target.WidgetButton ? Target.WidgetButton->GetCachedWidget() : TSharedPtr<SWidget>();
}

bool TriggerVirtualButtonClick(const FVirtualButtonTarget& Target)
{
#if WITH_ACCESSIBILITY
	if (Target.SlateButton.IsValid())
	{
		TWeakPtr<SWidget> WeakSlateButton = StaticCastSharedPtr<SWidget>(Target.SlateButton);
		FSlateAccessibleButton AccessibleButton(WeakSlateButton);
		AccessibleButton.Activate();
		return true;
	}
#endif

	if (Target.WidgetButton)
	{
		Target.WidgetButton->OnClicked.Broadcast();
		return true;
	}

	return false;
}
}

SDreamFlipCardBox::SDreamFlipCardBox()
{
	SetPixelSnapping(EWidgetPixelSnapping::Disabled);
}

SDreamFlipCardBox::~SDreamFlipCardBox()
{
	ReleaseRenderResources();
}

void SDreamFlipCardBox::Construct(const FArguments& InArgs)
{
	FrontFace.Widget = InArgs._FrontContent;
	BackFace.Widget = InArgs._BackContent;
	ChildSlot.AttachWidget(SNullWidget::NullWidget);
	SetCanTick(false);
}

void SDreamFlipCardBox::SetFrontContent(TSharedPtr<SWidget> InContent)
{
	FrontFace.Widget = InContent;
	ReleaseFaceResources(FrontFace);
	RequestRender();
	Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SDreamFlipCardBox::SetBackContent(TSharedPtr<SWidget> InContent)
{
	BackFace.Widget = InContent;
	ReleaseFaceResources(BackFace);
	RequestRender();
	Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SDreamFlipCardBox::SetFlipAngle(float InFlipAngle)
{
	if (!FMath::IsNearlyEqual(FlipAngle, InFlipAngle))
	{
		FlipAngle = InFlipAngle;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetHorizontalSegments(int32 InHorizontalSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InHorizontalSegments, 1, 128);
	if (HorizontalSegments != ClampedSegments)
	{
		HorizontalSegments = ClampedSegments;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetVerticalSegments(int32 InVerticalSegments)
{
	const int32 ClampedSegments = FMath::Clamp(InVerticalSegments, 1, 64);
	if (VerticalSegments != ClampedSegments)
	{
		VerticalSegments = ClampedSegments;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetPerspectiveStrength(float InPerspectiveStrength)
{
	const float ClampedStrength = FMath::Max(0.0f, InPerspectiveStrength);
	if (!FMath::IsNearlyEqual(PerspectiveStrength, ClampedStrength))
	{
		PerspectiveStrength = ClampedStrength;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetDepthOffset(float InDepthOffset)
{
	if (!FMath::IsNearlyEqual(DepthOffset, InDepthOffset))
	{
		DepthOffset = InDepthOffset;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetTransformPivot(FVector2D InTransformPivot)
{
	const FVector2D ClampedPivot(FMath::Clamp(InTransformPivot.X, 0.0, 1.0), FMath::Clamp(InTransformPivot.Y, 0.0, 1.0));
	if (!TransformPivot.Equals(ClampedPivot))
	{
		TransformPivot = ClampedPivot;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetRetainedRenderScale(float InRetainedRenderScale)
{
	const float ClampedScale = FMath::Max(0.1f, InRetainedRenderScale);
	if (!FMath::IsNearlyEqual(RetainedRenderScale, ClampedScale))
	{
		RetainedRenderScale = ClampedScale;
		RequestRender();
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamFlipCardBox::SetRenderEveryFrame(bool bInRenderEveryFrame)
{
	if (bRenderEveryFrame != bInRenderEveryFrame)
	{
		bRenderEveryFrame = bInRenderEveryFrame;
		RequestRender();
	}
}

void SDreamFlipCardBox::SetMirrorBackFace(bool bInMirrorBackFace)
{
	if (bMirrorBackFace != bInMirrorBackFace)
	{
		bMirrorBackFace = bInMirrorBackFace;
		InvalidateFlipCard();
	}
}

void SDreamFlipCardBox::SetOnHovered(FSimpleDelegate InOnHovered)
{
	OnHovered = MoveTemp(InOnHovered);
}

void SDreamFlipCardBox::SetOnUnhovered(FSimpleDelegate InOnUnhovered)
{
	OnUnhovered = MoveTemp(InOnUnhovered);
}

void SDreamFlipCardBox::RequestRender()
{
	FrontFace.bRenderRequested = true;
	BackFace.bRenderRequested = true;
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 SDreamFlipCardBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2f LocalSize = AllottedGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return LayerId;
	}

	const bool bBackFace = IsBackFaceVisible();
	FFaceState& VisibleFace = const_cast<SDreamFlipCardBox*>(this)->GetVisibleFace();
	UpdateFaceRenderTarget(VisibleFace, LocalSize, InWidgetStyle, bParentEnabled);
	if (!VisibleFace.RenderTarget.IsValid())
	{
		return LayerId;
	}

	VisibleFace.SurfaceBrush.SetResourceObject(VisibleFace.RenderTarget.Get());
	VisibleFace.SurfaceBrush.ImageSize = FVector2D(VisibleFace.RenderTargetSize);
	const FSlateResourceHandle ResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(VisibleFace.SurfaceBrush);
	if (!ResourceHandle.IsValid())
	{
		return LayerId;
	}

	TArray<FSlateVertex> Vertices;
	TArray<SlateIndex> Indices;
	BuildSlateMesh(AllottedGeometry, LocalSize, bBackFace, Vertices, Indices);
	if (Vertices.IsEmpty() || Indices.IsEmpty())
	{
		return LayerId;
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, ResourceHandle, Vertices, Indices, nullptr, 0, 0);
	return LayerId;
}

FVector2D SDreamFlipCardBox::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	FVector2D CombinedDesiredSize = FVector2D::ZeroVector;
	if (FrontFace.Widget.IsValid())
	{
		CombinedDesiredSize = CombinedDesiredSize.ComponentMax(FrontFace.Widget->GetDesiredSize());
	}
	if (BackFace.Widget.IsValid())
	{
		CombinedDesiredSize = CombinedDesiredSize.ComponentMax(BackFace.Widget->GetDesiredSize());
	}
	return CombinedDesiredSize;
}

void SDreamFlipCardBox::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnHovered.ExecuteIfBound();
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

FReply SDreamFlipCardBox::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ClearFaceVirtualHover(GetHiddenFace(), MouseEvent);

	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return RouteVirtualPointerDown(GetVisibleFace(), WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

FReply SDreamFlipCardBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ClearFaceVirtualHover(GetHiddenFace(), MouseEvent);

	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return RouteVirtualPointerUp(GetVisibleFace(), WidgetPath, RoutedEvent);
	}

	PressedVirtualButtonWidget.Reset();
	PressedVirtualButtonObject.Reset();
	ClearFaceVirtualHover(FrontFace, MouseEvent);
	ClearFaceVirtualHover(BackFace, MouseEvent);
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SDreamFlipCardBox::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ClearFaceVirtualHover(GetHiddenFace(), MouseEvent);

	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return RouteVirtualPointerMove(GetVisibleFace(), WidgetPath, RoutedEvent);
	}

	ClearFaceVirtualHover(GetVisibleFace(), MouseEvent);
	return FReply::Unhandled();
}

FReply SDreamFlipCardBox::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ClearFaceVirtualHover(GetHiddenFace(), MouseEvent);

	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RoutePointerDoubleClickEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

FReply SDreamFlipCardBox::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ClearFaceVirtualHover(GetHiddenFace(), MouseEvent);

	FPointerEvent RoutedEvent;
	FWidgetPath WidgetPath;
	if (BuildRoutedPointerEvent(MyGeometry, MouseEvent, RoutedEvent, WidgetPath))
	{
		return FSlateApplication::Get().RouteMouseWheelOrGestureEvent(WidgetPath, RoutedEvent);
	}

	return FReply::Unhandled();
}

void SDreamFlipCardBox::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	PressedVirtualButtonWidget.Reset();
	PressedVirtualButtonObject.Reset();
	ClearFaceVirtualHover(FrontFace, MouseEvent);
	ClearFaceVirtualHover(BackFace, MouseEvent);
	OnUnhovered.ExecuteIfBound();
	SCompoundWidget::OnMouseLeave(MouseEvent);
}

bool SDreamFlipCardBox::IsBackFaceVisible() const
{
	if (!BackFace.Widget.IsValid())
	{
		return false;
	}

	return FMath::Cos(FMath::DegreesToRadians(FlipAngle)) < 0.0f;
}

SDreamFlipCardBox::FFaceState& SDreamFlipCardBox::GetVisibleFace()
{
	return IsBackFaceVisible() ? BackFace : FrontFace;
}

const SDreamFlipCardBox::FFaceState& SDreamFlipCardBox::GetVisibleFace() const
{
	return IsBackFaceVisible() ? BackFace : FrontFace;
}

SDreamFlipCardBox::FFaceState& SDreamFlipCardBox::GetHiddenFace()
{
	return IsBackFaceVisible() ? FrontFace : BackFace;
}

const SDreamFlipCardBox::FFaceState& SDreamFlipCardBox::GetHiddenFace() const
{
	return IsBackFaceVisible() ? FrontFace : BackFace;
}

FVector2f SDreamFlipCardBox::ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance) const
{
	const float SafeDistance = FMath::Max(1.0f, CameraDistance - RotatedPoint.Z);
	const float ProjectedScale = CameraDistance / SafeDistance;

	return FVector2f(
		PivotPixels.X + RotatedPoint.X * ProjectedScale,
		PivotPixels.Y + RotatedPoint.Y * ProjectedScale);
}

void SDreamFlipCardBox::BuildSlateMesh(const FGeometry& AllottedGeometry, const FVector2f& LocalSize, bool bBackFace, TArray<FSlateVertex>& OutVertices, TArray<SlateIndex>& OutIndices) const
{
	const int32 SegmentCountX = FMath::Clamp(HorizontalSegments, 1, 128);
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
	const FQuat RotationQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(FlipAngle));
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
			const FVector LocalPoint(LocalX - PivotPixels.X, LocalY - PivotPixels.Y, DepthOffset);
			const FVector RotatedPoint = RotationQuat.RotateVector(LocalPoint);
			const FVector2f ProjectedPoint = ProjectRotatedPoint(RotatedPoint, PivotPixels, CameraDistance);

			FVector2f UV(U, V);
			if (bBackFace && bMirrorBackFace)
			{
				UV.X = 1.0f - UV.X;
			}

			OutVertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
				RenderTransform,
				ProjectedPoint,
				UV,
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

void SDreamFlipCardBox::UpdateFaceRenderTarget(FFaceState& Face, const FVector2f& LocalSize, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!Face.Widget.IsValid())
	{
		return;
	}

	const FIntPoint DesiredRenderTargetSize(
		FMath::Max(1, FMath::RoundToInt(LocalSize.X * RetainedRenderScale)),
		FMath::Max(1, FMath::RoundToInt(LocalSize.Y * RetainedRenderScale)));

	if (!Face.RenderTarget.IsValid() || Face.RenderTargetSize != DesiredRenderTargetSize)
	{
		Face.RenderTarget.Reset(FWidgetRenderer::CreateTargetFor(FVector2D(DesiredRenderTargetSize), TF_Bilinear, false));
		Face.RenderTargetSize = DesiredRenderTargetSize;
		Face.SurfaceBrush.SetResourceObject(Face.RenderTarget.Get());
		Face.SurfaceBrush.DrawAs = ESlateBrushDrawType::Image;
		Face.SurfaceBrush.Tiling = ESlateBrushTileType::NoTile;
		Face.SurfaceBrush.ImageSize = FVector2D(DesiredRenderTargetSize);
		Face.bRenderRequested = true;
	}

	if (!Face.RenderTarget.IsValid())
	{
		return;
	}

	if (!Face.WidgetRenderer.IsValid())
	{
		Face.WidgetRenderer = MakeUnique<FWidgetRenderer>(false, true);
	}

	if (bRenderEveryFrame || Face.bRenderRequested)
	{
		if (Face.Widget->GetVisibility().IsVisible())
		{
			const float DeltaTime = 0.0f;
			if (!Face.VirtualWindow.IsValid())
			{
				SAssignNew(Face.VirtualWindow, SVirtualWindow).Size(FVector2D(DesiredRenderTargetSize));
			}
			if (!Face.HitTestGrid.IsValid())
			{
				Face.HitTestGrid = MakeShared<FHittestGrid>();
			}

			const TSharedPtr<SWidget> OldParent = Face.Widget->GetParentWidget();
			Face.VirtualWindow->SetContent(Face.Widget.ToSharedRef());
			Face.VirtualWindow->Resize(FVector2D(DesiredRenderTargetSize));
			Face.WidgetRenderer->DrawWindow(Face.RenderTarget.Get(), *Face.HitTestGrid, Face.VirtualWindow.ToSharedRef(), 1.0f, FVector2D(DesiredRenderTargetSize), DeltaTime);
			if (OldParent.IsValid())
			{
				Face.Widget->AssignParentWidget(OldParent);
			}
		}
		Face.bRenderRequested = false;
	}
}

bool SDreamFlipCardBox::MapScreenPositionToVirtualWindow(const FGeometry& MyGeometry, const FVector2D& ScreenPosition, const FFaceState& Face, bool bBackFace, FVector2D& OutVirtualPosition) const
{
	if (Face.RenderTargetSize.X <= 0 || Face.RenderTargetSize.Y <= 0)
	{
		return false;
	}

	const FVector2f LocalSize = MyGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return false;
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(ScreenPosition);
	const int32 SegmentCountX = FMath::Clamp(HorizontalSegments, 1, 128);
	const int32 SegmentCountY = FMath::Clamp(VerticalSegments, 1, 64);
	const int32 VertexCountX = SegmentCountX + 1;
	const int32 VertexCountY = SegmentCountY + 1;
	const float LargestDimension = FMath::Max3(LocalSize.X, LocalSize.Y, 1.0f);
	const float CameraDistance = LargestDimension * FMath::Lerp(8.0f, 1.25f, FMath::Clamp(PerspectiveStrength, 0.0f, 1.0f));
	const FQuat RotationQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(FlipAngle));
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
			const FVector LocalPoint(LocalX - PivotPixels.X, LocalY - PivotPixels.Y, DepthOffset);
			const FVector RotatedPoint = RotationQuat.RotateVector(LocalPoint);
			const FVector2f ProjectedPoint = ProjectRotatedPoint(RotatedPoint, PivotPixels, CameraDistance);
			FVector2D UV(U, V);
			if (bBackFace && bMirrorBackFace)
			{
				UV.X = 1.0 - UV.X;
			}
			HitVertices.Add({FVector2D(ProjectedPoint), UV});
		}
	}

	auto TryTriangle = [&LocalPosition, &OutVirtualPosition, &Face](const FHitVertex& A, const FHitVertex& B, const FHitVertex& C) -> bool
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
		OutVirtualPosition = FVector2D(UV.X * Face.RenderTargetSize.X, UV.Y * Face.RenderTargetSize.Y);
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

bool SDreamFlipCardBox::BuildRoutedPointerEvent(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FPointerEvent& OutPointerEvent, FWidgetPath& OutWidgetPath) const
{
	const bool bBackFace = IsBackFaceVisible();
	const FFaceState& Face = GetVisibleFace();
	if (!Face.HitTestGrid.IsValid() || !Face.VirtualWindow.IsValid())
	{
		return false;
	}

	FVector2D VirtualPosition;
	if (!MapScreenPositionToVirtualWindow(MyGeometry, MouseEvent.GetScreenSpacePosition(), Face, bBackFace, VirtualPosition))
	{
		return false;
	}

	FVector2D LastVirtualPosition;
	if (!MapScreenPositionToVirtualWindow(MyGeometry, MouseEvent.GetLastScreenSpacePosition(), Face, bBackFace, LastVirtualPosition))
	{
		LastVirtualPosition = VirtualPosition;
	}

	TArray<FWidgetAndPointer> BubblePath = Face.HitTestGrid->GetBubblePath(VirtualPosition, 0.0f, false, MouseEvent.GetUserIndex());
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

void SDreamFlipCardBox::UpdateVirtualHoverPath(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const
{
	TArray<TWeakPtr<SWidget>> NewHoverWidgets;
	NewHoverWidgets.Reserve(NewWidgetPath.Widgets.Num());
	for (int32 WidgetIndex = 0; WidgetIndex < NewWidgetPath.Widgets.Num(); ++WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
		if (!ArrangedWidget.Widget->Advanced_IsWindow())
		{
			NewHoverWidgets.Add(ArrangedWidget.Widget);
		}
	}

	for (int32 PreviousIndex = Face.LastVirtualHoverWidgets.Num() - 1; PreviousIndex >= 0; --PreviousIndex)
	{
		const TSharedPtr<SWidget> PreviousWidget = Face.LastVirtualHoverWidgets[PreviousIndex].Pin();
		if (!PreviousWidget.IsValid())
		{
			continue;
		}

		const bool bStillHovered = NewHoverWidgets.ContainsByPredicate([&PreviousWidget](const TWeakPtr<SWidget>& NewWidget)
		{
			return NewWidget.Pin() == PreviousWidget;
		});

		if (!bStillHovered)
		{
			PreviousWidget->OnMouseLeave(MouseEvent);
		}
	}

	for (int32 WidgetIndex = 0; WidgetIndex < NewWidgetPath.Widgets.Num(); ++WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
		if (ArrangedWidget.Widget->Advanced_IsWindow())
		{
			continue;
		}

		const bool bWasHovered = Face.LastVirtualHoverWidgets.ContainsByPredicate([&ArrangedWidget](const TWeakPtr<SWidget>& PreviousWidget)
		{
			return PreviousWidget.Pin() == ArrangedWidget.Widget;
		});

		if (!bWasHovered)
		{
			ArrangedWidget.Widget->OnMouseEnter(ArrangedWidget.Geometry, MouseEvent);
		}
	}

	Face.LastVirtualHoverWidgets = MoveTemp(NewHoverWidgets);
	Face.bHasVirtualHover = Face.LastVirtualHoverWidgets.Num() > 0;
}

FReply SDreamFlipCardBox::RouteVirtualPointerDown(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent)
{
	UpdateVirtualHoverPath(Face, NewWidgetPath, MouseEvent);

	PressedVirtualButtonWidget.Reset();
	PressedVirtualButtonObject.Reset();
	const FVirtualButtonTarget PressedButton = FindVirtualButtonInPath(NewWidgetPath);
	if (TSharedPtr<SWidget> PressedButtonWidget = GetVirtualButtonWidget(PressedButton))
	{
		PressedVirtualButtonObject = PressedButton.WidgetButton;
		PressedVirtualButtonWidget = PressedButtonWidget;
	}

	FReply Reply = FReply::Unhandled();
	for (int32 WidgetIndex = 0; WidgetIndex < NewWidgetPath.Widgets.Num(); ++WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
		if (ArrangedWidget.Widget->Advanced_IsWindow())
		{
			continue;
		}

		Reply = ArrangedWidget.Widget->OnPreviewMouseButtonDown(ArrangedWidget.Geometry, MouseEvent);
		if (Reply.IsEventHandled())
		{
			break;
		}
	}

	if (!Reply.IsEventHandled())
	{
		for (int32 WidgetIndex = NewWidgetPath.Widgets.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
		{
			const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
			if (ArrangedWidget.Widget->Advanced_IsWindow())
			{
				continue;
			}

			if (MouseEvent.IsTouchEvent())
			{
				Reply = ArrangedWidget.Widget->OnTouchStarted(ArrangedWidget.Geometry, MouseEvent);
			}

			if (!MouseEvent.IsTouchEvent() || !Reply.IsEventHandled())
			{
				Reply = ArrangedWidget.Widget->OnMouseButtonDown(ArrangedWidget.Geometry, MouseEvent);
			}

			if (Reply.IsEventHandled())
			{
				break;
			}
		}
	}

	if (Reply.IsEventHandled())
	{
		Reply.CaptureMouse(AsShared());
	}

	return Reply;
}

FReply SDreamFlipCardBox::RouteVirtualPointerUp(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent)
{
	UpdateVirtualHoverPath(Face, NewWidgetPath, MouseEvent);

	const FVirtualButtonTarget ReleasedButton = FindVirtualButtonInPath(NewWidgetPath);
	const bool bClickMethodNeedsManualDispatch = ReleasedButton.WidgetButton
		? ReleasedButton.WidgetButton->GetClickMethod() == EButtonClickMethod::DownAndUp
		: true;
	const TSharedPtr<SWidget> ReleasedButtonWidget = GetVirtualButtonWidget(ReleasedButton);
	const bool bShouldEmitVirtualClick = ReleasedButtonWidget.IsValid()
		&& ReleasedButtonWidget == PressedVirtualButtonWidget.Pin()
		&& (!PressedVirtualButtonObject.IsValid() || ReleasedButton.WidgetButton == PressedVirtualButtonObject.Get())
		&& bClickMethodNeedsManualDispatch
		&& (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.IsTouchEvent());

	FReply Reply = FReply::Unhandled();
	for (int32 WidgetIndex = NewWidgetPath.Widgets.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
		if (ArrangedWidget.Widget->Advanced_IsWindow())
		{
			continue;
		}

		if (MouseEvent.IsTouchEvent())
		{
			Reply = ArrangedWidget.Widget->OnTouchEnded(ArrangedWidget.Geometry, MouseEvent);
		}

		if (!MouseEvent.IsTouchEvent() || !Reply.IsEventHandled())
		{
			Reply = ArrangedWidget.Widget->OnMouseButtonUp(ArrangedWidget.Geometry, MouseEvent);
		}

		if (Reply.IsEventHandled())
		{
			break;
		}
	}

	if (bShouldEmitVirtualClick && TriggerVirtualButtonClick(ReleasedButton))
	{
		Reply = FReply::Handled();
	}

	PressedVirtualButtonWidget.Reset();
	PressedVirtualButtonObject.Reset();

	if (Reply.IsEventHandled())
	{
		Reply.ReleaseMouseCapture();
	}

	return Reply;
}

FReply SDreamFlipCardBox::RouteVirtualPointerMove(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const
{
	UpdateVirtualHoverPath(Face, NewWidgetPath, MouseEvent);

	FReply Reply = FReply::Unhandled();
	for (int32 WidgetIndex = NewWidgetPath.Widgets.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		const FArrangedWidget& ArrangedWidget = NewWidgetPath.Widgets[WidgetIndex];
		if (ArrangedWidget.Widget->Advanced_IsWindow())
		{
			continue;
		}

		Reply = ArrangedWidget.Widget->OnMouseMove(ArrangedWidget.Geometry, MouseEvent);
		if (Reply.IsEventHandled())
		{
			break;
		}
	}

	return Reply;
}

void SDreamFlipCardBox::ClearFaceVirtualHover(FFaceState& Face, const FPointerEvent& MouseEvent) const
{
	if (!Face.bHasVirtualHover || Face.bClearingVirtualHover)
	{
		return;
	}

	TGuardValue<bool> ClearingGuard(Face.bClearingVirtualHover, true);
	Face.bHasVirtualHover = false;
	for (int32 PreviousIndex = Face.LastVirtualHoverWidgets.Num() - 1; PreviousIndex >= 0; --PreviousIndex)
	{
		if (const TSharedPtr<SWidget> PreviousWidget = Face.LastVirtualHoverWidgets[PreviousIndex].Pin())
		{
			PreviousWidget->OnMouseLeave(MouseEvent);
		}
	}
	Face.LastVirtualHoverWidgets.Reset();
}

void SDreamFlipCardBox::ReleaseFaceResources(FFaceState& Face) const
{
	Face.bRenderRequested = true;
	Face.bHasVirtualHover = false;
	Face.bClearingVirtualHover = false;
	Face.LastVirtualHoverWidgets.Reset();
	Face.WidgetRenderer.Reset();
	Face.VirtualWindow.Reset();
	Face.HitTestGrid.Reset();
	Face.RenderTarget.Reset();
	Face.RenderTargetSize = FIntPoint::ZeroValue;
}

void SDreamFlipCardBox::ReleaseRenderResources()
{
	PressedVirtualButtonWidget.Reset();
	PressedVirtualButtonObject.Reset();
	ReleaseFaceResources(FrontFace);
	ReleaseFaceResources(BackFace);
}

void SDreamFlipCardBox::InvalidateFlipCard()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
