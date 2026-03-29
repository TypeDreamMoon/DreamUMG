// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextWidgetTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateColor.h"
#include "Widgets/DreamTextAnimation.h"
#include "DreamAnimatedTextBlock.generated.h"

class SDreamAnimatedTextBlock;

UCLASS(meta = (DisplayName = "Dream Animated Text"))
class DREAMUMG_API UDreamAnimatedTextBlock : public UTextLayoutWidget
{
	GENERATED_BODY()

public:
	UDreamAnimatedTextBlock(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateColor ColorAndOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float MinDesiredWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (DisplayName = "Shadow Color"))
	FLinearColor ShadowColorAndOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FVector2D ShadowOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (DisplayName = "Transform Policy"))
	ETextTransformPolicy TextTransformPolicy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	int32 VisibleCharacterCount;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Animation", meta = (DisplayName = "Animation Player"))
	TObjectPtr<UDreamTextAnimationPlayerBase> AnimationPlayer;

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetColorAndOpacity(FSlateColor InColorAndOpacity);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetFont(FSlateFontInfo InFont);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetMinDesiredWidth(float InMinDesiredWidth);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetShadowColorAndOpacity(FLinearColor InShadowColorAndOpacity);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetShadowOffset(FVector2D InShadowOffset);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetTextTransformPolicy(ETextTransformPolicy InTransformPolicy);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetVisibleCharacterCount(int32 InVisibleCharacterCount);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetTypewriterPlayer(UDreamTextTypewriterPlayer* InTypewriterPlayer);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetAnimationPlayer(UDreamTextAnimationPlayerBase* InAnimationPlayer);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ResetAnimationState();

	virtual void PostLoad() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual void OnShapedTextOptionsChanged(FShapedTextOptions InShapedTextOptions) override;
	virtual void OnJustificationChanged(ETextJustify::Type InJustification) override;
	virtual void OnWrappingPolicyChanged(ETextWrappingPolicy InWrappingPolicy) override;
	virtual void OnAutoWrapTextChanged(bool InAutoWrapText) override;
	virtual void OnWrapTextAtChanged(float InWrapTextAt) override;
	virtual void OnLineHeightPercentageChanged(float InLineHeightPercentage) override;
	virtual void OnApplyLineHeightToBottomLineChanged(bool InApplyLineHeightToBottomLine) override;
	virtual void OnMarginChanged(const FMargin& InMargin) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void PushAllPropertiesToSlate();

	UPROPERTY()
	TObjectPtr<UDreamTextTypewriterPlayer> TypewriterPlayer;

	TSharedPtr<SDreamAnimatedTextBlock> MyAnimatedTextBlock;
};
