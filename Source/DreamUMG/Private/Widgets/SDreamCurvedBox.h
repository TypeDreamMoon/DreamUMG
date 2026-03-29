// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/ArrangedChildren.h"
#include "Widgets/SCompoundWidget.h"

class SDreamCurvedBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamCurvedBox)
		: _Content()
	{
	}

		SLATE_DEFAULT_SLOT(FArguments, Content)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetContent(const TSharedRef<SWidget>& InContent);
	void SetRotation(FRotator InRotation);
	void SetCurveAngle(float InCurveAngle);
	void SetCurveSegments(int32 InCurveSegments);
	void SetPerspectiveStrength(float InPerspectiveStrength);
	void SetDepthOffset(float InDepthOffset);
	void SetTransformPivot(FVector2D InTransformPivot);

protected:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

private:
	FVector2f ProjectLocalPoint(const FVector2f& LocalPoint, const FVector2f& LocalSize, float CameraDistance, const FQuat& RotationQuat) const;

	FRotator Rotation = FRotator::ZeroRotator;
	float CurveAngle = 45.0f;
	int32 CurveSegments = 24;
	float PerspectiveStrength = 0.35f;
	float DepthOffset = 0.0f;
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);
};
