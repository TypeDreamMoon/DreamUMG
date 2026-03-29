// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/DreamAnimatedTextBlock.h"

#include "Widgets/SDreamAnimatedTextBlock.h"

#define LOCTEXT_NAMESPACE "DreamAnimatedTextBlock"

UDreamAnimatedTextBlock::UDreamAnimatedTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Text = FText::FromString(TEXT("Dream Animated Text"));
	ColorAndOpacity = FSlateColor(FLinearColor::White);
	MinDesiredWidth = 0.0f;
	ShadowColorAndOpacity = FLinearColor(0.0f, 0.0f, 0.0f, 0.75f);
	ShadowOffset = FVector2D::ZeroVector;
	TextTransformPolicy = ETextTransformPolicy::None;
	VisibleCharacterCount = -1;

	Justification = ETextJustify::Left;
	WrappingPolicy = ETextWrappingPolicy::DefaultWrapping;
	AutoWrapText = false;
	ApplyLineHeightToBottomLine = true;
	WrapTextAt = 0.0f;
	Margin = FMargin(0.0f);
	LineHeightPercentage = 1.0f;
}

void UDreamAnimatedTextBlock::SetText(const FText& InText)
{
	Text = InText;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetText(Text);
	}
}

void UDreamAnimatedTextBlock::SetColorAndOpacity(FSlateColor InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetColorAndOpacity(ColorAndOpacity);
	}
}

void UDreamAnimatedTextBlock::SetFont(FSlateFontInfo InFont)
{
	Font = MoveTemp(InFont);
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetFont(Font);
	}
}

void UDreamAnimatedTextBlock::SetMinDesiredWidth(float InMinDesiredWidth)
{
	MinDesiredWidth = InMinDesiredWidth;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetMinDesiredWidth(MinDesiredWidth);
	}
}

void UDreamAnimatedTextBlock::SetShadowColorAndOpacity(FLinearColor InShadowColorAndOpacity)
{
	ShadowColorAndOpacity = InShadowColorAndOpacity;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetShadowColorAndOpacity(ShadowColorAndOpacity);
	}
}

void UDreamAnimatedTextBlock::SetShadowOffset(FVector2D InShadowOffset)
{
	ShadowOffset = InShadowOffset;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetShadowOffset(ShadowOffset);
	}
}

void UDreamAnimatedTextBlock::SetTextTransformPolicy(ETextTransformPolicy InTransformPolicy)
{
	TextTransformPolicy = InTransformPolicy;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetTextTransformPolicy(TextTransformPolicy);
	}
}

void UDreamAnimatedTextBlock::SetVisibleCharacterCount(int32 InVisibleCharacterCount)
{
	VisibleCharacterCount = InVisibleCharacterCount;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetVisibleCharacterCount(VisibleCharacterCount);
	}
}

void UDreamAnimatedTextBlock::PostLoad()
{
	Super::PostLoad();

	if (AnimationPlayer == nullptr && TypewriterPlayer != nullptr)
	{
		AnimationPlayer = TypewriterPlayer;
	}

	TypewriterPlayer = nullptr;
}

void UDreamAnimatedTextBlock::SetTypewriterPlayer(UDreamTextTypewriterPlayer* InTypewriterPlayer)
{
	SetAnimationPlayer(InTypewriterPlayer);
}

void UDreamAnimatedTextBlock::SetAnimationPlayer(UDreamTextAnimationPlayerBase* InAnimationPlayer)
{
	AnimationPlayer = InAnimationPlayer;
	TypewriterPlayer = nullptr;
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetAnimationPlayer(AnimationPlayer);
	}
}

void UDreamAnimatedTextBlock::ResetAnimationState()
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->ResetAnimationState();
	}
}

void UDreamAnimatedTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushAllPropertiesToSlate();
}

void UDreamAnimatedTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyAnimatedTextBlock.Reset();
}

#if WITH_EDITOR
const FText UDreamAnimatedTextBlock::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}
#endif

void UDreamAnimatedTextBlock::OnShapedTextOptionsChanged(FShapedTextOptions InShapedTextOptions)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetTextShapingMethod(InShapedTextOptions.bOverride_TextShapingMethod ? TOptional<ETextShapingMethod>(InShapedTextOptions.TextShapingMethod) : TOptional<ETextShapingMethod>());
		MyAnimatedTextBlock->SetTextFlowDirection(InShapedTextOptions.bOverride_TextFlowDirection ? TOptional<ETextFlowDirection>(InShapedTextOptions.TextFlowDirection) : TOptional<ETextFlowDirection>());
	}
}

void UDreamAnimatedTextBlock::OnJustificationChanged(ETextJustify::Type InJustification)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetJustification(InJustification);
	}
}

void UDreamAnimatedTextBlock::OnWrappingPolicyChanged(ETextWrappingPolicy InWrappingPolicy)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetWrappingPolicy(InWrappingPolicy);
	}
}

void UDreamAnimatedTextBlock::OnAutoWrapTextChanged(bool InAutoWrapText)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetAutoWrapText(InAutoWrapText);
	}
}

void UDreamAnimatedTextBlock::OnWrapTextAtChanged(float InWrapTextAt)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetWrapTextAt(InWrapTextAt);
	}
}

void UDreamAnimatedTextBlock::OnLineHeightPercentageChanged(float InLineHeightPercentage)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetLineHeightPercentage(InLineHeightPercentage);
	}
}

void UDreamAnimatedTextBlock::OnApplyLineHeightToBottomLineChanged(bool InApplyLineHeightToBottomLine)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetApplyLineHeightToBottomLine(InApplyLineHeightToBottomLine);
	}
}

void UDreamAnimatedTextBlock::OnMarginChanged(const FMargin& InMargin)
{
	if (MyAnimatedTextBlock.IsValid())
	{
		MyAnimatedTextBlock->SetMargin(InMargin);
	}
}

TSharedRef<SWidget> UDreamAnimatedTextBlock::RebuildWidget()
{
	SAssignNew(MyAnimatedTextBlock, SDreamAnimatedTextBlock);
	PushAllPropertiesToSlate();
	return MyAnimatedTextBlock.ToSharedRef();
}

void UDreamAnimatedTextBlock::PushAllPropertiesToSlate()
{
	if (!MyAnimatedTextBlock.IsValid())
	{
		return;
	}

	MyAnimatedTextBlock->SetText(Text);
	MyAnimatedTextBlock->SetFont(Font);
	MyAnimatedTextBlock->SetColorAndOpacity(ColorAndOpacity);
	MyAnimatedTextBlock->SetMinDesiredWidth(MinDesiredWidth);
	MyAnimatedTextBlock->SetShadowColorAndOpacity(ShadowColorAndOpacity);
	MyAnimatedTextBlock->SetShadowOffset(ShadowOffset);
	MyAnimatedTextBlock->SetTextTransformPolicy(TextTransformPolicy);
	MyAnimatedTextBlock->SetVisibleCharacterCount(VisibleCharacterCount);
	MyAnimatedTextBlock->SetAnimationPlayer(AnimationPlayer);
	MyAnimatedTextBlock->SetTextShapingMethod(ShapedTextOptions.bOverride_TextShapingMethod ? TOptional<ETextShapingMethod>(ShapedTextOptions.TextShapingMethod) : TOptional<ETextShapingMethod>());
	MyAnimatedTextBlock->SetTextFlowDirection(ShapedTextOptions.bOverride_TextFlowDirection ? TOptional<ETextFlowDirection>(ShapedTextOptions.TextFlowDirection) : TOptional<ETextFlowDirection>());
	MyAnimatedTextBlock->SetJustification(Justification);
	MyAnimatedTextBlock->SetWrappingPolicy(WrappingPolicy);
	MyAnimatedTextBlock->SetAutoWrapText(AutoWrapText);
	MyAnimatedTextBlock->SetWrapTextAt(WrapTextAt);
	MyAnimatedTextBlock->SetLineHeightPercentage(LineHeightPercentage);
	MyAnimatedTextBlock->SetApplyLineHeightToBottomLine(ApplyLineHeightToBottomLine);
	MyAnimatedTextBlock->SetMargin(Margin);
}

#undef LOCTEXT_NAMESPACE
