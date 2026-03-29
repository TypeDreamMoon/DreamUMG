// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/SDreamAnimatedTextBlock.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/TextTransformer.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Slate/WidgetTransform.h"

namespace DreamAnimatedTextBlock
{
	static constexpr TCHAR SpaceCharacter = TEXT(' ');
	static constexpr TCHAR TabCharacter = TEXT('\t');
	static constexpr TCHAR NewLineCharacter = TEXT('\n');
	static constexpr TCHAR CarriageReturnCharacter = TEXT('\r');

	static bool IsWhitespace(TCHAR Character)
	{
		return Character == SpaceCharacter || Character == TabCharacter;
	}

	static FString ExpandCharacter(TCHAR Character)
	{
		if (Character == TabCharacter)
		{
			return TEXT("    ");
		}
		return FString::Chr(Character);
	}
}

void SDreamAnimatedTextBlock::Construct(const FArguments& InArgs)
{
	SetCanTick(false);
	bCanSupportFocus = false;
	ResetAnimationState();
}

void SDreamAnimatedTextBlock::SetText(const FText& InText)
{
	if (!Text.EqualTo(InText))
	{
		Text = InText;
		InvalidateLayout();
		ResetAnimationState();
	}
}

void SDreamAnimatedTextBlock::SetFont(const FSlateFontInfo& InFont)
{
	Font = InFont;
	InvalidateLayout();
}

void SDreamAnimatedTextBlock::SetColorAndOpacity(const FSlateColor& InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SDreamAnimatedTextBlock::SetMinDesiredWidth(float InMinDesiredWidth)
{
	MinDesiredWidth = InMinDesiredWidth;
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SDreamAnimatedTextBlock::SetShadowColorAndOpacity(const FLinearColor& InShadowColorAndOpacity)
{
	ShadowColorAndOpacity = InShadowColorAndOpacity;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SDreamAnimatedTextBlock::SetShadowOffset(const FVector2D& InShadowOffset)
{
	ShadowOffset = InShadowOffset;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SDreamAnimatedTextBlock::SetTextTransformPolicy(ETextTransformPolicy InTextTransformPolicy)
{
	if (TextTransformPolicy != InTextTransformPolicy)
	{
		TextTransformPolicy = InTextTransformPolicy;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetVisibleCharacterCount(int32 InVisibleCharacterCount)
{
	if (VisibleCharacterCount != InVisibleCharacterCount)
	{
		VisibleCharacterCount = InVisibleCharacterCount;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamAnimatedTextBlock::SetAnimationPlayer(UDreamTextAnimationPlayerBase* InAnimationPlayer)
{
	AnimationPlayer = InAnimationPlayer;
	ResetAnimationState();
}

void SDreamAnimatedTextBlock::ResetAnimationState()
{
	if (UDreamTextAnimationPlayerBase* Player = AnimationPlayer.Get())
	{
		Player->ResetRuntimeState();
	}

	RestartAnimationTimerIfNeeded();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SDreamAnimatedTextBlock::SetTextShapingMethod(TOptional<ETextShapingMethod> InTextShapingMethod)
{
	TextShapingMethod = InTextShapingMethod;
}

void SDreamAnimatedTextBlock::SetTextFlowDirection(TOptional<ETextFlowDirection> InTextFlowDirection)
{
	TextFlowDirection = InTextFlowDirection;
}

void SDreamAnimatedTextBlock::SetJustification(ETextJustify::Type InJustification)
{
	if (Justification != InJustification)
	{
		Justification = InJustification;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SDreamAnimatedTextBlock::SetWrappingPolicy(ETextWrappingPolicy InWrappingPolicy)
{
	if (WrappingPolicy != InWrappingPolicy)
	{
		WrappingPolicy = InWrappingPolicy;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetAutoWrapText(bool bInAutoWrapText)
{
	if (bAutoWrapText != bInAutoWrapText)
	{
		bAutoWrapText = bInAutoWrapText;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetWrapTextAt(float InWrapTextAt)
{
	if (!FMath::IsNearlyEqual(WrapTextAt, InWrapTextAt))
	{
		WrapTextAt = InWrapTextAt;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetLineHeightPercentage(float InLineHeightPercentage)
{
	if (!FMath::IsNearlyEqual(LineHeightPercentage, InLineHeightPercentage))
	{
		LineHeightPercentage = InLineHeightPercentage;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetApplyLineHeightToBottomLine(bool bInApplyLineHeightToBottomLine)
{
	if (bApplyLineHeightToBottomLine != bInApplyLineHeightToBottomLine)
	{
		bApplyLineHeightToBottomLine = bInApplyLineHeightToBottomLine;
		InvalidateLayout();
	}
}

void SDreamAnimatedTextBlock::SetMargin(const FMargin& InMargin)
{
	if (Margin != InMargin)
	{
		Margin = InMargin;
		Invalidate(EInvalidateWidgetReason::Layout);
	}
}

int32 SDreamAnimatedTextBlock::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const_cast<SDreamAnimatedTextBlock*>(this)->RestartAnimationTimerIfNeeded();

	const FCachedLayout& Layout = GetOrBuildLayout(GetAvailableTextWidth(AllottedGeometry));
	if (Layout.Glyphs.IsEmpty())
	{
		return LayerId;
	}

	const bool bEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FLinearColor BaseColor = InWidgetStyle.GetColorAndOpacityTint() * ColorAndOpacity.GetColor(InWidgetStyle);
	const FLinearColor ShadowColor = InWidgetStyle.GetColorAndOpacityTint() * ShadowColorAndOpacity;
	const FVector2f MarginOffset(static_cast<float>(Margin.Left), static_cast<float>(Margin.Top));
	const float AvailableTextWidth = GetAvailableTextWidth(AllottedGeometry);
	const int32 EffectiveVisibleCharacters = GetEffectiveVisibleCharacterCount(Layout.TotalVisibleCharacters);
	const bool bDrawShadow = !ShadowOffset.IsNearlyZero() && ShadowColor.A > KINDA_SMALL_NUMBER;

	for (const FLineLayout& Line : Layout.Lines)
	{
		float JustificationOffset = 0.0f;
		switch (Justification)
		{
		case ETextJustify::Center:
			JustificationOffset = (AvailableTextWidth - Line.Width) * 0.5f;
			break;
		case ETextJustify::Right:
			JustificationOffset = AvailableTextWidth - Line.Width;
			break;
		case ETextJustify::Left:
		default:
			break;
		}

		for (int32 GlyphIndex = Line.StartGlyphIndex; GlyphIndex < Line.EndGlyphIndex; ++GlyphIndex)
		{
			const FGlyphLayout& Glyph = Layout.Glyphs[GlyphIndex];
			if (Glyph.RevealOrdinal >= EffectiveVisibleCharacters)
			{
				continue;
			}

			const FDreamTextAnimationGlyphState AnimatedState = BuildAnimatedGlyphState(Glyph, Layout, BaseColor);
			const FVector2f FinalPosition = MarginOffset + FVector2f(JustificationOffset, 0.0f) + AnimatedState.Position;
			const FVector2f SafeScale(
				FMath::IsNearlyZero(AnimatedState.Scale.X) ? KINDA_SMALL_NUMBER : AnimatedState.Scale.X,
				FMath::IsNearlyZero(AnimatedState.Scale.Y) ? KINDA_SMALL_NUMBER : AnimatedState.Scale.Y
			);

			const FSlateRenderTransform RenderTransform = FWidgetTransform(
				FVector2D::ZeroVector,
				FVector2D(SafeScale),
				FVector2D::ZeroVector,
				AnimatedState.Rotation
			).ToSlateRenderTransform();

			const FLinearColor FinalColor(
				AnimatedState.Color.R,
				AnimatedState.Color.G,
				AnimatedState.Color.B,
				AnimatedState.Color.A * AnimatedState.Opacity
			);

			if (bDrawShadow && Glyph.bDrawGlyph)
			{
				const FPaintGeometry ShadowGeometry = AllottedGeometry.ToPaintGeometry(
					Glyph.Size,
					FSlateLayoutTransform(FinalPosition + FVector2f(ShadowOffset)),
					RenderTransform,
					FVector2f(0.5f, 0.5f)
				);

				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId,
					ShadowGeometry,
					Glyph.GlyphText,
					Font,
					DrawEffects,
					FLinearColor(ShadowColor.R, ShadowColor.G, ShadowColor.B, ShadowColor.A * AnimatedState.Opacity)
				);
			}

			if (Glyph.bDrawGlyph)
			{
				const FPaintGeometry GlyphGeometry = AllottedGeometry.ToPaintGeometry(
					Glyph.Size,
					FSlateLayoutTransform(FinalPosition),
					RenderTransform,
					FVector2f(0.5f, 0.5f)
				);

				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId,
					GlyphGeometry,
					Glyph.GlyphText,
					Font,
					DrawEffects,
					FinalColor
				);
			}
		}
	}

	return LayerId;
}

FVector2D SDreamAnimatedTextBlock::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	const FCachedLayout& Layout = GetOrBuildLayout(0.0f);
	return FVector2D(
		FMath::Max(MinDesiredWidth, Layout.DesiredSize.X + Margin.GetTotalSpaceAlong<Orient_Horizontal>()),
		Layout.DesiredSize.Y + Margin.GetTotalSpaceAlong<Orient_Vertical>()
	);
}

bool SDreamAnimatedTextBlock::ComputeVolatility() const
{
	return IsAnyRuntimeAnimationActive();
}

void SDreamAnimatedTextBlock::InvalidateLayout()
{
	CachedLayout.bValid = false;
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SDreamAnimatedTextBlock::RestartAnimationTimerIfNeeded()
{
	const bool bShouldRunTimer = IsAnyRuntimeAnimationActive();
	const TSharedPtr<FActiveTimerHandle> PinnedHandle = ActiveTimerHandle.Pin();

	if (bShouldRunTimer)
	{
		if (!PinnedHandle.IsValid())
		{
			ActiveTimerHandle = RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SDreamAnimatedTextBlock::HandleAnimationTick));
		}
	}
	else if (PinnedHandle.IsValid())
	{
		UnRegisterActiveTimer(PinnedHandle.ToSharedRef());
		ActiveTimerHandle.Reset();
	}
}

bool SDreamAnimatedTextBlock::IsAnyRuntimeAnimationActive() const
{
	const UDreamTextAnimationPlayerBase* Player = AnimationPlayer.Get();
	return Player != nullptr && Player->RequiresTick();
}

FDreamTextAnimationLayoutContext SDreamAnimatedTextBlock::BuildAnimationLayoutContext(const FCachedLayout& Layout) const
{
	FDreamTextAnimationLayoutContext LayoutContext;
	LayoutContext.TotalAnimatedCharacters = Layout.TotalVisibleCharacters;
	LayoutContext.TotalVisibleCharacters = Layout.TotalVisibleCharacters;
	return LayoutContext;
}

const SDreamAnimatedTextBlock::FCachedLayout& SDreamAnimatedTextBlock::GetOrBuildLayout(float AvailableWidth) const
{
	const float EffectiveWrapWidth = GetEffectiveWrapWidth(AvailableWidth);
	const FString DisplayString = GetDisplayString();

	if (!CachedLayout.bValid
		|| !CachedLayout.DisplayText.Equals(DisplayString, ESearchCase::CaseSensitive)
		|| !FMath::IsNearlyEqual(CachedLayout.WrapWidth, EffectiveWrapWidth)
		|| !FMath::IsNearlyEqual(CachedLayout.LineHeightPercentage, LineHeightPercentage)
		|| CachedLayout.bApplyLineHeightToBottomLine != bApplyLineHeightToBottomLine
		|| CachedLayout.Font != Font)
	{
		BuildLayout(CachedLayout, AvailableWidth);
	}

	return CachedLayout;
}

void SDreamAnimatedTextBlock::BuildLayout(FCachedLayout& Layout, float AvailableWidth) const
{
	Layout = FCachedLayout();
	Layout.DisplayText = GetDisplayString();
	Layout.WrapWidth = GetEffectiveWrapWidth(AvailableWidth);
	Layout.LineHeightPercentage = LineHeightPercentage;
	Layout.bApplyLineHeightToBottomLine = bApplyLineHeightToBottomLine;
	Layout.Font = Font;
	Layout.bValid = true;

	if (!FSlateApplication::IsInitialized() || FSlateApplication::Get().GetRenderer() == nullptr)
	{
		return;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float MaxCharacterHeight = static_cast<float>(FontMeasure->GetMaxCharacterHeight(Font, 1.0f));
	const float EffectiveLineHeight = FMath::Max(MaxCharacterHeight * FMath::Max(LineHeightPercentage, KINDA_SMALL_NUMBER), KINDA_SMALL_NUMBER);
	const float EffectiveWrapWidth = Layout.WrapWidth;

	float CurrentX = 0.0f;
	float CurrentY = 0.0f;
	float MaxLineWidth = 0.0f;
	int32 CurrentLineIndex = 0;
	int32 CurrentLineStartGlyphIndex = 0;
	int32 VisibleCharacterOrdinal = 0;
	TCHAR PreviousCharacter = 0;
	bool bHasPreviousCharacterOnLine = false;

	const auto FinalizeLine = [&Layout, &MaxLineWidth, &CurrentLineStartGlyphIndex](const float LineWidth, const int32 EndGlyphIndex)
	{
		FLineLayout& Line = Layout.Lines.AddDefaulted_GetRef();
		Line.StartGlyphIndex = CurrentLineStartGlyphIndex;
		Line.EndGlyphIndex = EndGlyphIndex;
		Line.Width = LineWidth;
		MaxLineWidth = FMath::Max(MaxLineWidth, LineWidth);
		CurrentLineStartGlyphIndex = EndGlyphIndex;
	};

	const FString& DisplayString = Layout.DisplayText;
	for (int32 Index = 0; Index < DisplayString.Len(); ++Index)
	{
		const TCHAR Character = DisplayString[Index];
		if (Character == DreamAnimatedTextBlock::CarriageReturnCharacter)
		{
			continue;
		}

		if (Character == DreamAnimatedTextBlock::NewLineCharacter)
		{
			FinalizeLine(CurrentX, Layout.Glyphs.Num());
			CurrentX = 0.0f;
			CurrentY += EffectiveLineHeight;
			PreviousCharacter = 0;
			bHasPreviousCharacterOnLine = false;
			++CurrentLineIndex;
			continue;
		}

		const FString GlyphText = DreamAnimatedTextBlock::ExpandCharacter(Character);
		const bool bWhitespace = DreamAnimatedTextBlock::IsWhitespace(Character);
		const FVector2f GlyphSize = FontMeasure->Measure(FStringView(GlyphText), Font, 1.0f);
		const float Kerning = bHasPreviousCharacterOnLine ? static_cast<float>(FontMeasure->GetKerning(Font, 1.0f, PreviousCharacter, Character)) : 0.0f;
		const float ProposedX = CurrentX + Kerning;

		if (EffectiveWrapWidth > 0.0f && ProposedX > 0.0f && (ProposedX + GlyphSize.X) > EffectiveWrapWidth)
		{
			FinalizeLine(CurrentX, Layout.Glyphs.Num());
			CurrentX = 0.0f;
			CurrentY += EffectiveLineHeight;
			PreviousCharacter = 0;
			bHasPreviousCharacterOnLine = false;
			++CurrentLineIndex;
		}

		const float FinalKerning = bHasPreviousCharacterOnLine ? static_cast<float>(FontMeasure->GetKerning(Font, 1.0f, PreviousCharacter, Character)) : 0.0f;
		CurrentX += FinalKerning;

		FGlyphLayout& Glyph = Layout.Glyphs.AddDefaulted_GetRef();
		Glyph.GlyphText = GlyphText;
		Glyph.Position = FVector2f(CurrentX, CurrentY);
		Glyph.Size = FVector2f(GlyphSize.X, MaxCharacterHeight);
		Glyph.LineIndex = CurrentLineIndex;
		Glyph.RevealOrdinal = VisibleCharacterOrdinal;
		Glyph.AnimatedIndex = bWhitespace ? INDEX_NONE : Layout.TotalVisibleCharacters;
		Glyph.bDrawGlyph = !bWhitespace;
		Glyph.bWhitespace = bWhitespace;

		if (!bWhitespace)
		{
			++VisibleCharacterOrdinal;
			++Layout.TotalVisibleCharacters;
		}

		CurrentX += GlyphSize.X;
		PreviousCharacter = Character;
		bHasPreviousCharacterOnLine = true;
	}

	if (Layout.Lines.IsEmpty() || Layout.Lines.Last().EndGlyphIndex != Layout.Glyphs.Num())
	{
		FinalizeLine(CurrentX, Layout.Glyphs.Num());
	}

	const int32 LineCount = FMath::Max(Layout.Lines.Num(), 1);
	const float DesiredHeight = bApplyLineHeightToBottomLine
		? EffectiveLineHeight * static_cast<float>(LineCount)
		: (EffectiveLineHeight * static_cast<float>(FMath::Max(LineCount - 1, 0))) + MaxCharacterHeight;

	Layout.DesiredSize = FVector2f(MaxLineWidth, DesiredHeight);
}

FString SDreamAnimatedTextBlock::GetDisplayString() const
{
	FString DisplayString = Text.ToString();
	switch (TextTransformPolicy)
	{
	case ETextTransformPolicy::ToLower:
		return FTextTransformer::ToLower(DisplayString);
	case ETextTransformPolicy::ToUpper:
		return FTextTransformer::ToUpper(DisplayString);
	case ETextTransformPolicy::None:
	default:
		return DisplayString;
	}
}

float SDreamAnimatedTextBlock::GetEffectiveWrapWidth(float AvailableWidth) const
{
	if (WrapTextAt > 0.0f)
	{
		return WrapTextAt;
	}

	if (bAutoWrapText && AvailableWidth > 0.0f)
	{
		return FMath::Max(0.0f, AvailableWidth);
	}

	return 0.0f;
}

float SDreamAnimatedTextBlock::GetAvailableTextWidth(const FGeometry& AllottedGeometry) const
{
	return FMath::Max(0.0f, static_cast<float>(AllottedGeometry.GetLocalSize().X) - Margin.GetTotalSpaceAlong<Orient_Horizontal>());
}

int32 SDreamAnimatedTextBlock::GetEffectiveVisibleCharacterCount(int32 TotalVisibleCharacters) const
{
	int32 EffectiveVisibleCharacters = VisibleCharacterCount < 0
		? TotalVisibleCharacters
		: FMath::Clamp(VisibleCharacterCount, 0, TotalVisibleCharacters);

	if (const UDreamTextAnimationPlayerBase* Player = AnimationPlayer.Get())
	{
		EffectiveVisibleCharacters = FMath::Min(EffectiveVisibleCharacters, Player->GetVisibleCharacterCount(TotalVisibleCharacters));
	}

	return FMath::Clamp(EffectiveVisibleCharacters, 0, TotalVisibleCharacters);
}

FDreamTextAnimationGlyphState SDreamAnimatedTextBlock::BuildAnimatedGlyphState(const FGlyphLayout& Glyph, const FCachedLayout& Layout, const FLinearColor& BaseColor) const
{
	FDreamTextAnimationGlyphState State;
	State.Position = Glyph.Position;
	State.Color = BaseColor;

	FDreamTextAnimationGlyphContext GlyphContext;
	GlyphContext.AnimatedIndex = Glyph.AnimatedIndex;
	GlyphContext.TotalAnimatedCharacters = Layout.TotalVisibleCharacters;
	GlyphContext.RevealOrdinal = Glyph.RevealOrdinal;
	GlyphContext.TotalVisibleCharacters = Layout.TotalVisibleCharacters;

	const UDreamTextAnimationPlayerBase* Player = AnimationPlayer.Get();
	if (Player == nullptr)
	{
		State.Scale.X = FMath::Max(State.Scale.X, KINDA_SMALL_NUMBER);
		State.Scale.Y = FMath::Max(State.Scale.Y, KINDA_SMALL_NUMBER);
		State.Opacity = FMath::Clamp(State.Opacity, 0.0f, 1.0f);
		return State;
	}

	Player->ApplyToGlyph(GlyphContext, State);

	State.Scale.X = FMath::Max(State.Scale.X, KINDA_SMALL_NUMBER);
	State.Scale.Y = FMath::Max(State.Scale.Y, KINDA_SMALL_NUMBER);
	State.Opacity = FMath::Clamp(State.Opacity, 0.0f, 1.0f);
	return State;
}

EActiveTimerReturnType SDreamAnimatedTextBlock::HandleAnimationTick(double CurrentTime, float DeltaTime)
{
	UDreamTextAnimationPlayerBase* Player = AnimationPlayer.Get();
	const bool bAnimationActive = Player != nullptr && Player->RequiresTick();

	if (!bAnimationActive)
	{
		ActiveTimerHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}

	bool bKeepTicking = false;
	if (bAnimationActive)
	{
		const FCachedLayout& Layout = GetOrBuildLayout(0.0f);
		const FDreamTextAnimationLayoutContext LayoutContext = BuildAnimationLayoutContext(Layout);
		bKeepTicking |= Player->TickPlayer(DeltaTime, LayoutContext);
	}

	Invalidate(EInvalidateWidgetReason::Paint);

	if (!bKeepTicking)
	{
		ActiveTimerHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}
