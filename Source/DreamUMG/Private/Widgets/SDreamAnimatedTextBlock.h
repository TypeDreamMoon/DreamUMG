// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Text/TextLayout.h"
#include "Layout/Margin.h"
#include "Styling/SlateColor.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DreamTextAnimation.h"

class SDreamAnimatedTextBlock : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamAnimatedTextBlock)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetText(const FText& InText);
	void SetFont(const FSlateFontInfo& InFont);
	void SetColorAndOpacity(const FSlateColor& InColorAndOpacity);
	void SetMinDesiredWidth(float InMinDesiredWidth);
	void SetShadowColorAndOpacity(const FLinearColor& InShadowColorAndOpacity);
	void SetShadowOffset(const FVector2D& InShadowOffset);
	void SetTextTransformPolicy(ETextTransformPolicy InTextTransformPolicy);
	void SetVisibleCharacterCount(int32 InVisibleCharacterCount);
	void SetAnimationPlayer(UDreamTextAnimationPlayerBase* InAnimationPlayer);
	void ResetAnimationState();

	void SetTextShapingMethod(TOptional<ETextShapingMethod> InTextShapingMethod);
	void SetTextFlowDirection(TOptional<ETextFlowDirection> InTextFlowDirection);
	void SetJustification(ETextJustify::Type InJustification);
	void SetWrappingPolicy(ETextWrappingPolicy InWrappingPolicy);
	void SetAutoWrapText(bool bInAutoWrapText);
	void SetWrapTextAt(float InWrapTextAt);
	void SetLineHeightPercentage(float InLineHeightPercentage);
	void SetApplyLineHeightToBottomLine(bool bInApplyLineHeightToBottomLine);
	void SetMargin(const FMargin& InMargin);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual bool ComputeVolatility() const override;

private:
	struct FGlyphLayout
	{
		FString GlyphText;
		FVector2f Position = FVector2f::ZeroVector;
		FVector2f Size = FVector2f::ZeroVector;
		int32 LineIndex = 0;
		int32 RevealOrdinal = 0;
		int32 AnimatedIndex = INDEX_NONE;
		bool bDrawGlyph = true;
		bool bWhitespace = false;
	};

	struct FLineLayout
	{
		int32 StartGlyphIndex = 0;
		int32 EndGlyphIndex = 0;
		float Width = 0.0f;
	};

	struct FCachedLayout
	{
		bool bValid = false;
		FString DisplayText;
		float WrapWidth = 0.0f;
		float LineHeightPercentage = 1.0f;
		bool bApplyLineHeightToBottomLine = true;
		FSlateFontInfo Font;
		FVector2f DesiredSize = FVector2f::ZeroVector;
		int32 TotalVisibleCharacters = 0;
		TArray<FGlyphLayout> Glyphs;
		TArray<FLineLayout> Lines;
	};

	void InvalidateLayout();
	void RestartAnimationTimerIfNeeded();
	bool IsAnyRuntimeAnimationActive() const;
	FDreamTextAnimationLayoutContext BuildAnimationLayoutContext(const FCachedLayout& Layout) const;

	const FCachedLayout& GetOrBuildLayout(float AvailableWidth) const;
	void BuildLayout(FCachedLayout& Layout, float AvailableWidth) const;
	FString GetDisplayString() const;
	float GetEffectiveWrapWidth(float AvailableWidth) const;
	float GetAvailableTextWidth(const FGeometry& AllottedGeometry) const;
	int32 GetEffectiveVisibleCharacterCount(int32 TotalVisibleCharacters) const;
	FDreamTextAnimationGlyphState BuildAnimatedGlyphState(const FGlyphLayout& Glyph, const FCachedLayout& Layout, const FLinearColor& BaseColor) const;
	EActiveTimerReturnType HandleAnimationTick(double CurrentTime, float DeltaTime);

private:
	FText Text;
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);
	FSlateFontInfo Font;
	float MinDesiredWidth = 0.0f;
	FLinearColor ShadowColorAndOpacity = FLinearColor(ForceInit);
	FVector2D ShadowOffset = FVector2D::ZeroVector;
	ETextTransformPolicy TextTransformPolicy = ETextTransformPolicy::None;
	int32 VisibleCharacterCount = -1;
	TWeakObjectPtr<UDreamTextAnimationPlayerBase> AnimationPlayer;

	TOptional<ETextShapingMethod> TextShapingMethod;
	TOptional<ETextFlowDirection> TextFlowDirection;
	ETextJustify::Type Justification = ETextJustify::Left;
	ETextWrappingPolicy WrappingPolicy = ETextWrappingPolicy::DefaultWrapping;
	bool bAutoWrapText = false;
	float WrapTextAt = 0.0f;
	float LineHeightPercentage = 1.0f;
	bool bApplyLineHeightToBottomLine = true;
	FMargin Margin;

	mutable FCachedLayout CachedLayout;
	mutable TWeakPtr<FActiveTimerHandle> ActiveTimerHandle;
};
