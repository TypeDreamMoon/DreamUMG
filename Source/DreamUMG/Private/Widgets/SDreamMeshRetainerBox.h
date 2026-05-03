// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/ArrangedChildren.h"
#include "Rendering/SlateRenderTransform.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class FWidgetRenderer;
class FHittestGrid;
class SVirtualWindow;
class UButton;
class UTextureRenderTarget2D;

class SDreamMeshRetainerBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamMeshRetainerBox)
		: _Content()
	{
	}

		SLATE_DEFAULT_SLOT(FArguments, Content)

	SLATE_END_ARGS()

	SDreamMeshRetainerBox();
	virtual ~SDreamMeshRetainerBox() override;

	void Construct(const FArguments& InArgs);

	void SetContent(const TSharedRef<SWidget>& InContent);
	void SetRotation(FRotator InRotation);
	void SetCurveAngle(float InCurveAngle);
	void SetCurveSegments(int32 InCurveSegments);
	void SetVerticalSegments(int32 InVerticalSegments);
	void SetPerspectiveStrength(float InPerspectiveStrength);
	void SetDepthOffset(float InDepthOffset);
	void SetTransformPivot(FVector2D InTransformPivot);
	void SetRetainedRenderScale(float InRetainedRenderScale);
	void SetRenderEveryFrame(bool bInRenderEveryFrame);
	void RequestRender();

protected:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	FVector BuildCurvedPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize) const;
	FVector2f ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance) const;
	void BuildSlateMesh(const FGeometry& AllottedGeometry, const FVector2f& LocalSize, TArray<FSlateVertex>& OutVertices, TArray<SlateIndex>& OutIndices) const;
	void UpdateRenderTarget(const FVector2f& LocalSize, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	bool MapScreenPositionToVirtualWindow(const FGeometry& MyGeometry, const FVector2D& ScreenPosition, FVector2D& OutVirtualPosition) const;
	bool BuildRoutedPointerEvent(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FPointerEvent& OutPointerEvent, FWidgetPath& OutWidgetPath) const;
	void UpdateVirtualHoverPath(const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const;
	FReply RouteVirtualPointerDown(const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent);
	FReply RouteVirtualPointerUp(const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent);
	FReply RouteVirtualPointerMove(const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const;
	void ClearVirtualHover(const FPointerEvent& MouseEvent) const;
	void ReleaseRenderResources();
	void InvalidateMeshRetainer();

	FRotator Rotation = FRotator::ZeroRotator;
	float CurveAngle = 45.0f;
	int32 CurveSegments = 48;
	int32 VerticalSegments = 1;
	float PerspectiveStrength = 0.35f;
	float DepthOffset = 0.0f;
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);
	float RetainedRenderScale = 1.0f;
	bool bRenderEveryFrame = true;

	mutable bool bRenderRequested = true;
	mutable bool bHasVirtualHover = false;
	mutable bool bClearingVirtualHover = false;
	mutable TArray<TWeakPtr<SWidget>> LastVirtualHoverWidgets;
	TWeakPtr<SWidget> PressedVirtualButtonWidget;
	TWeakObjectPtr<UButton> PressedVirtualButtonObject;
	mutable FIntPoint RenderTargetSize = FIntPoint::ZeroValue;
	mutable TUniquePtr<FWidgetRenderer> WidgetRenderer;
	mutable TSharedPtr<SVirtualWindow> VirtualWindow;
	mutable TSharedPtr<FHittestGrid> HitTestGrid;
	mutable TStrongObjectPtr<UTextureRenderTarget2D> RenderTarget;
	mutable FSlateBrush SurfaceBrush;
};
