// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "DreamPseudo3DBox.generated.h"

class SDreamPseudo3DBox;

UCLASS(meta = (DisplayName = "Dream Pseudo 3D Box"))
class DREAMUMG_API UDreamPseudo3DBox : public UContentWidget
{
	GENERATED_BODY()

public:
	UDreamPseudo3DBox(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetRotation, Category = "Pseudo 3D")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetPerspectiveStrength, Category = "Pseudo 3D", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float PerspectiveStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetFieldOfView, Category = "Pseudo 3D", meta = (ClampMin = "1.0", ClampMax = "170.0", UIMin = "1.0", UIMax = "120.0"))
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetDepthOffset, Category = "Pseudo 3D")
	float DepthOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetPerspectiveSegments, Category = "Pseudo 3D", meta = (ClampMin = "1", ClampMax = "12", UIMin = "1", UIMax = "8"))
	int32 PerspectiveSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, BlueprintSetter = SetTransformPivot, Category = "Pseudo 3D", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector2D TransformPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pseudo 3D")
	bool bIgnoreClipping;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetRotation(FRotator InRotation);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetPerspectiveStrength(float InPerspectiveStrength);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetFieldOfView(float InFieldOfView);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetDepthOffset(float InDepthOffset);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetPerspectiveSegments(int32 InPerspectiveSegments);

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Pseudo 3D")
	void SetTransformPivot(FVector2D InTransformPivot);

	UFUNCTION(BlueprintCallable, Category = "Pseudo 3D")
	void SetIgnoreClipping(bool bInIgnoreClipping);

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

	TSharedPtr<SDreamPseudo3DBox> MyPseudo3DBox;
};
