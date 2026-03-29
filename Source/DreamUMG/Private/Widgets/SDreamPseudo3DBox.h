// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/ArrangedChildren.h"
#include "Rendering/SlateRenderTransform.h"
#include "Widgets/SCompoundWidget.h"

class SDreamPseudo3DBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamPseudo3DBox)
		: _Content()
	{
	}

		SLATE_DEFAULT_SLOT(FArguments, Content)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetContent(const TSharedRef<SWidget>& InContent);
	void SetRotation(FRotator InRotation);
	void SetPerspectiveStrength(float InPerspectiveStrength);
	void SetFieldOfView(float InFieldOfView);
	void SetDepthOffset(float InDepthOffset);
	void SetPerspectiveSegments(int32 InPerspectiveSegments);
	void SetTransformPivot(FVector2D InTransformPivot);
	void SetIgnoreClipping(bool bInIgnoreClipping);

protected:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

private:
	void InvalidatePseudo3DTransform();
	FVector2f ProjectLocalPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize, float CameraDistance, const FQuat& RotationQuat) const;

	FRotator Rotation = FRotator::ZeroRotator;
	float PerspectiveStrength = 0.35f;
	float FieldOfView = 45.0f;
	float DepthOffset = 0.0f;
	int32 PerspectiveSegments = 6;
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);
	bool bIgnoreClipping = true;
};
