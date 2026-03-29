// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RHIResources.h"
#include "ScreenPass.h"
#include "SceneViewExtension.h"
#include "Components/WidgetComponent.h"

struct FDreamWidgetRenderItem
{
	uint32 ComponentId = 0;
	FTransform Transform = FTransform::Identity;
	FVector2D Size = FVector2D::ZeroVector;
	FVector2D Pivot = FVector2D(0.5f, 0.5f);
	EWidgetGeometryMode GeometryMode = EWidgetGeometryMode::Plane;
	float CylinderArcAngle = 180.0f;
	bool bVisible = false;
	bool bTwoSided = true;
	int32 SortPriority = 0;
	FTextureRHIRef Texture;
};

class FDreamWidgetViewExtension : public FSceneViewExtensionBase
{
public:
	FDreamWidgetViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo) override {}
	virtual void SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SubscribeToPostProcessingPass(EPostProcessingPass Pass, const FSceneView& InView, FPostProcessingPassDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;
	virtual int32 GetPriority() const override;
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;

	void UpsertRenderItem(const FDreamWidgetRenderItem& InRenderItem);
	void RemoveRenderItem(uint32 ComponentId);
	bool HasRenderItems() const;

private:
	FScreenPassTexture PostProcessPassAfterRender_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& Inputs);

	mutable FCriticalSection RenderItemsCS;
	TMap<uint32, FDreamWidgetRenderItem> RenderItems;
	TWeakObjectPtr<UWorld> World;
};

namespace DreamWidgetViewExtensionRegistry
{
	TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> Get(UWorld* World);
	TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> Find(UWorld* World);
}
