// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PanelWidget.h"
#include "DreamFlipCardBox.generated.h"

class SDreamFlipCardBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDreamFlipCardHoverEvent);

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dream Flip Card Box"))
class DREAMUMG_API UDreamFlipCardBox : public UPanelWidget
{
	GENERATED_BODY()

public:
	UDreamFlipCardBox(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetFlipAngle, Category = "Flip Card", meta = (UIMin = "0.0", UIMax = "360.0"))
	float FlipAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetHorizontalSegments, Category = "Flip Card", meta = (ClampMin = "1", ClampMax = "128", UIMin = "1", UIMax = "64"))
	int32 HorizontalSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetVerticalSegments, Category = "Flip Card", meta = (ClampMin = "1", ClampMax = "64", UIMin = "1", UIMax = "16"))
	int32 VerticalSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetPerspectiveStrength, Category = "Flip Card", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float PerspectiveStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetDepthOffset, Category = "Flip Card")
	float DepthOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetTransformPivot, Category = "Flip Card", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector2D TransformPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetRetainedRenderScale, Category = "Flip Card", meta = (ClampMin = "0.1", UIMin = "0.25", UIMax = "2.0"))
	float RetainedRenderScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetRenderEveryFrame, Category = "Flip Card")
	bool bRenderEveryFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetMirrorBackFace, Category = "Flip Card")
	bool bMirrorBackFace;

	UPROPERTY(BlueprintAssignable, Category = "Flip Card|Event")
	FDreamFlipCardHoverEvent OnCardHovered;

	UPROPERTY(BlueprintAssignable, Category = "Flip Card|Event")
	FDreamFlipCardHoverEvent OnCardUnhovered;

	UPROPERTY(BlueprintAssignable, Category = "Flip Card|Event")
	FDreamFlipCardHoverEvent OnCardClicked;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetFlipAngle(float InFlipAngle);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetHorizontalSegments(int32 InHorizontalSegments);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetVerticalSegments(int32 InVerticalSegments);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetPerspectiveStrength(float InPerspectiveStrength);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetDepthOffset(float InDepthOffset);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetTransformPivot(FVector2D InTransformPivot);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetRetainedRenderScale(float InRetainedRenderScale);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetRenderEveryFrame(bool bInRenderEveryFrame);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Flip Card")
	void SetMirrorBackFace(bool bInMirrorBackFace);

	UFUNCTION(BlueprintCallable, Category = "Flip Card")
	void RequestRender();

	UFUNCTION(BlueprintPure, Category = "Flip Card")
	UWidget* GetFrontWidget() const;

	UFUNCTION(BlueprintPure, Category = "Flip Card")
	UWidget* GetBackWidget() const;

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;

private:
	void HandleSlateHovered();
	void HandleSlateUnhovered();
	void HandleSlateClicked();
	void PushAllPropertiesToSlate();
	void RefreshChildren();

	TSharedPtr<SDreamFlipCardBox> MyFlipCardBox;
};
