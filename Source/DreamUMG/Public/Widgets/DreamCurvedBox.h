// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "DreamCurvedBox.generated.h"

class SDreamCurvedBox;

UCLASS(meta = (DisplayName = "Dream Curved Box"))
class DREAMUMG_API UDreamCurvedBox : public UContentWidget
{
	GENERATED_BODY()

public:
	UDreamCurvedBox(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved", meta = (UIMin = "-180.0", UIMax = "180.0"))
	float CurveAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved", meta = (ClampMin = "2", UIMin = "2", UIMax = "64"))
	int32 CurveSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float PerspectiveStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved")
	float DepthOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curved", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector2D TransformPivot;

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetRotation(FRotator InRotation);

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetCurveAngle(float InCurveAngle);

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetCurveSegments(int32 InCurveSegments);

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetPerspectiveStrength(float InPerspectiveStrength);

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetDepthOffset(float InDepthOffset);

	UFUNCTION(BlueprintCallable, Category = "Curved")
	void SetTransformPivot(FVector2D InTransformPivot);

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

	TSharedPtr<SDreamCurvedBox> MyCurvedBox;
};
