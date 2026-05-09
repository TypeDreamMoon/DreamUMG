// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/SlateRenderTransform.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class FWidgetRenderer;
class FHittestGrid;
class SVirtualWindow;
class UButton;
class UTextureRenderTarget2D;

class SDreamFlipCardBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamFlipCardBox)
		: _FrontContent()
		, _BackContent()
	{
	}

		SLATE_ARGUMENT(TSharedPtr<SWidget>, FrontContent)
		SLATE_ARGUMENT(TSharedPtr<SWidget>, BackContent)

	SLATE_END_ARGS()

	SDreamFlipCardBox();
	virtual ~SDreamFlipCardBox() override;

	void Construct(const FArguments& InArgs);

	void SetFrontContent(TSharedPtr<SWidget> InContent);
	void SetBackContent(TSharedPtr<SWidget> InContent);
	void SetFlipAngle(float InFlipAngle);
	void SetHorizontalSegments(int32 InHorizontalSegments);
	void SetVerticalSegments(int32 InVerticalSegments);
	void SetPerspectiveStrength(float InPerspectiveStrength);
	void SetDepthOffset(float InDepthOffset);
	void SetTransformPivot(FVector2D InTransformPivot);
	void SetRetainedRenderScale(float InRetainedRenderScale);
	void SetRenderEveryFrame(bool bInRenderEveryFrame);
	void SetMirrorBackFace(bool bInMirrorBackFace);
	void SetOnHovered(FSimpleDelegate InOnHovered);
	void SetOnUnhovered(FSimpleDelegate InOnUnhovered);
	void SetOnClicked(FSimpleDelegate InOnClicked);
	void RequestRender();

protected:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	struct FFaceState
	{
		TSharedPtr<SWidget> Widget;
		mutable bool bRenderRequested = true;
		mutable bool bHasVirtualHover = false;
		mutable bool bClearingVirtualHover = false;
		mutable TArray<TWeakPtr<SWidget>> LastVirtualHoverWidgets;
		mutable FIntPoint RenderTargetSize = FIntPoint::ZeroValue;
		mutable TUniquePtr<FWidgetRenderer> WidgetRenderer;
		mutable TSharedPtr<SVirtualWindow> VirtualWindow;
		mutable TSharedPtr<FHittestGrid> HitTestGrid;
		mutable TStrongObjectPtr<UTextureRenderTarget2D> RenderTarget;
		mutable FSlateBrush SurfaceBrush;
	};

	bool IsBackFaceVisible() const;
	FFaceState& GetVisibleFace();
	const FFaceState& GetVisibleFace() const;
	FFaceState& GetHiddenFace();
	const FFaceState& GetHiddenFace() const;
	FVector2f ProjectRotatedPoint(const FVector& RotatedPoint, const FVector2f& PivotPixels, float CameraDistance) const;
	void BuildSlateMesh(const FGeometry& AllottedGeometry, const FVector2f& LocalSize, bool bBackFace, TArray<FSlateVertex>& OutVertices, TArray<SlateIndex>& OutIndices) const;
	void UpdateFaceRenderTarget(FFaceState& Face, const FVector2f& LocalSize, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	bool MapScreenPositionToVirtualWindow(const FGeometry& MyGeometry, const FVector2D& ScreenPosition, const FFaceState& Face, bool bBackFace, FVector2D& OutVirtualPosition) const;
	bool BuildRoutedPointerEvent(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FPointerEvent& OutPointerEvent, FWidgetPath& OutWidgetPath) const;
	void UpdateVirtualHoverPath(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const;
	FReply RouteVirtualPointerDown(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent);
	FReply RouteVirtualPointerUp(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent);
	FReply RouteVirtualPointerMove(FFaceState& Face, const FWidgetPath& NewWidgetPath, const FPointerEvent& MouseEvent) const;
	void ClearFaceVirtualHover(FFaceState& Face, const FPointerEvent& MouseEvent) const;
	void ReleaseFaceResources(FFaceState& Face) const;
	void ReleaseRenderResources();
	void InvalidateFlipCard();

	float FlipAngle = 0.0f;
	int32 HorizontalSegments = 24;
	int32 VerticalSegments = 1;
	float PerspectiveStrength = 0.35f;
	float DepthOffset = 0.0f;
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);
	float RetainedRenderScale = 1.0f;
	bool bRenderEveryFrame = true;
	bool bMirrorBackFace = true;

	FSimpleDelegate OnHovered;
	FSimpleDelegate OnUnhovered;
	FSimpleDelegate OnClicked;
	FFaceState FrontFace;
	FFaceState BackFace;
	TWeakPtr<SWidget> PressedVirtualButtonWidget;
	TWeakObjectPtr<UButton> PressedVirtualButtonObject;
};
