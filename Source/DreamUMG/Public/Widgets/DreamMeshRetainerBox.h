// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "DreamMeshRetainerBox.generated.h"

class SDreamMeshRetainerBox;

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dream Mesh Retainer Box"))
class DREAMUMG_API UDreamMeshRetainerBox : public UContentWidget
{
	GENERATED_BODY()

public:
	UDreamMeshRetainerBox(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetRotation, Category = "Mesh Retainer")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetCurveAngle, Category = "Mesh Retainer", meta = (UIMin = "-180.0", UIMax = "180.0"))
	float CurveAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetCurveSegments, Category = "Mesh Retainer", meta = (ClampMin = "2", ClampMax = "256", UIMin = "2", UIMax = "128"))
	int32 CurveSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetVerticalSegments, Category = "Mesh Retainer", meta = (ClampMin = "1", ClampMax = "64", UIMin = "1", UIMax = "16"))
	int32 VerticalSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetPerspectiveStrength, Category = "Mesh Retainer", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float PerspectiveStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetDepthOffset, Category = "Mesh Retainer")
	float DepthOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetTransformPivot, Category = "Mesh Retainer", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector2D TransformPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetRetainedRenderScale, Category = "Mesh Retainer", meta = (ClampMin = "0.1", UIMin = "0.25", UIMax = "2.0"))
	float RetainedRenderScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetRenderEveryFrame, Category = "Mesh Retainer")
	bool bRenderEveryFrame;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetRotation(FRotator InRotation);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetCurveAngle(float InCurveAngle);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetCurveSegments(int32 InCurveSegments);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetVerticalSegments(int32 InVerticalSegments);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetPerspectiveStrength(float InPerspectiveStrength);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetDepthOffset(float InDepthOffset);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetTransformPivot(FVector2D InTransformPivot);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetRetainedRenderScale(float InRetainedRenderScale);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Mesh Retainer")
	void SetRenderEveryFrame(bool bInRenderEveryFrame);

	UFUNCTION(BlueprintCallable, Category = "Mesh Retainer")
	void RequestRender();

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;

private:
	void PushAllPropertiesToSlate();
	void RefreshContent();

	TSharedPtr<SDreamMeshRetainerBox> MyMeshRetainerBox;
};
