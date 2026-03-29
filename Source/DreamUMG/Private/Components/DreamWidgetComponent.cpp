// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/DreamWidgetComponent.h"

#include "Components/DreamWidgetViewExtension.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Widgets/SNullWidget.h"

UDreamWidgetComponent::UDreamWidgetComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(true);
	SetEditTimeUsable(true);
	SetTickMode(ETickMode::Enabled);
	SetTwoSided(true);
	SetBlendMode(EWidgetBlendMode::Transparent);
	SetBackgroundColor(FLinearColor::Transparent);
	SetTickWhenOffscreen(true);
}

void UDreamWidgetComponent::SetRootWidget(UDreamSlate3DWidget* InRootWidget)
{
	RootWidget = InRootWidget;
	RebuildSlateWidget();
}

void UDreamWidgetComponent::RebuildSlateWidget()
{
	if (RootWidget != nullptr)
	{
		BuiltSlateWidget = RootWidget->BuildWidget();
		SetSlateWidget(BuiltSlateWidget);
	}
	else if (BuiltSlateWidget.IsValid())
	{
		BuiltSlateWidget.Reset();
		SetSlateWidget(nullptr);
	}

	RequestRenderUpdate();
	UpdateRendererEntry();
}

FPrimitiveSceneProxy* UDreamWidgetComponent::CreateSceneProxy()
{
	return nullptr;
}

void UDreamWidgetComponent::OnRegister()
{
	Super::OnRegister();
	RebuildSlateWidget();
	UpdateRendererEntry();
}

void UDreamWidgetComponent::OnUnregister()
{
	RemoveRendererEntry();
	Super::OnUnregister();
}

void UDreamWidgetComponent::DestroyComponent(bool bPromoteChildren)
{
	RemoveRendererEntry();
	Super::DestroyComponent(bPromoteChildren);
}

void UDreamWidgetComponent::PostLoad()
{
	Super::PostLoad();
	RebuildSlateWidget();
	UpdateRendererEntry();
}

void UDreamWidgetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateRendererEntry();
}

void UDreamWidgetComponent::SetWidget(UUserWidget* InWidget)
{
	Super::SetWidget(InWidget);
	UpdateRendererEntry();
}

void UDreamWidgetComponent::SetSlateWidget(const TSharedPtr<SWidget>& InSlateWidget)
{
	Super::SetSlateWidget(InSlateWidget);
	UpdateRendererEntry();
}

bool UDreamWidgetComponent::ShouldDrawWidget() const
{
	if (!IsVisible())
	{
		return false;
	}

	if ((GetCurrentTime() - LastWidgetRenderTime) >= RedrawTime)
	{
		return bManuallyRedraw ? bRedrawRequested : true;
	}

	return false;
}

void UDreamWidgetComponent::OnHiddenInGameChanged()
{
	Super::OnHiddenInGameChanged();
	UpdateRendererEntry();
}

void UDreamWidgetComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	UpdateRendererEntry();
}

#if WITH_EDITOR
void UDreamWidgetComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildSlateWidget();
	UpdateRendererEntry();
}
#endif

void UDreamWidgetComponent::UpdateRendererEntry()
{
	if (!IsRegistered())
	{
		return;
	}

	UTextureRenderTarget2D* WidgetRenderTarget = GetRenderTarget();
	if (WidgetRenderTarget == nullptr && (GetUserWidgetObject() != nullptr || GetSlateWidget().IsValid() || RootWidget != nullptr))
	{
		UpdateWidget();
		DrawWidgetToRenderTarget(0.0f);
		WidgetRenderTarget = GetRenderTarget();
	}

	if (WidgetRenderTarget == nullptr || !IsVisible() || !IsWidgetVisible())
	{
		RemoveRendererEntry();
		return;
	}

	FTextureRenderTargetResource* RenderTargetResource = WidgetRenderTarget->GameThread_GetRenderTargetResource();
	if (RenderTargetResource == nullptr)
	{
		RemoveRendererEntry();
		return;
	}

	FTextureRHIRef RenderTargetTexture = RenderTargetResource->GetRenderTargetTexture();
	if (!RenderTargetTexture.IsValid())
	{
		RemoveRendererEntry();
		return;
	}

	FDreamWidgetRenderItem RenderItem;
	RenderItem.ComponentId = GetUniqueID();
	RenderItem.Transform = GetComponentTransform();
	RenderItem.Size = FVector2D(WidgetRenderTarget->SizeX, WidgetRenderTarget->SizeY);
	RenderItem.Pivot = GetPivot();
	RenderItem.GeometryMode = GetGeometryMode();
	RenderItem.CylinderArcAngle = static_cast<float>(GetCylinderArcAngle());
	RenderItem.bVisible = true;
	RenderItem.bTwoSided = GetTwoSided();
	RenderItem.SortPriority = TranslucencySortPriority;
	RenderItem.Texture = RenderTargetTexture;

	if (RenderItem.Size.GetMin() <= 0.0f)
	{
		RemoveRendererEntry();
		return;
	}

	if (TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> ViewExtension = DreamWidgetViewExtensionRegistry::Get(GetWorld()))
	{
		ViewExtension->UpsertRenderItem(RenderItem);
	}
}

void UDreamWidgetComponent::RemoveRendererEntry()
{
	if (TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> ViewExtension = DreamWidgetViewExtensionRegistry::Find(GetWorld()))
	{
		ViewExtension->RemoveRenderItem(GetUniqueID());
	}
}
