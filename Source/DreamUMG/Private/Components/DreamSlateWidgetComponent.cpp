// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/DreamSlateWidgetComponent.h"

#include "Widgets/SNullWidget.h"

UDreamSlateWidgetComponent::UDreamSlateWidgetComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(true);
	SetTwoSided(true);
}

void UDreamSlateWidgetComponent::SetRootWidget(UDreamSlate3DWidget* InRootWidget)
{
	RootWidget = InRootWidget;
	RebuildSlateWidget();
}

void UDreamSlateWidgetComponent::RebuildSlateWidget()
{
	BuiltSlateWidget = RootWidget != nullptr
		? RootWidget->BuildWidget()
		: StaticCastSharedRef<SWidget>(SNullWidget::NullWidget);

	SetSlateWidget(BuiltSlateWidget);
	RequestRenderUpdate();
}

void UDreamSlateWidgetComponent::OnRegister()
{
	Super::OnRegister();
	RebuildSlateWidget();
}

void UDreamSlateWidgetComponent::PostLoad()
{
	Super::PostLoad();
	RebuildSlateWidget();
}

#if WITH_EDITOR
void UDreamSlateWidgetComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildSlateWidget();
}
#endif
