// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/DreamSlateHostWidget.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DreamSlateHostWidget"

UDreamSlateHostWidget::UDreamSlateHostWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDreamSlateHostWidget::SetRootWidget(UDreamSlate3DWidget* InRootWidget)
{
	RootWidget = InRootWidget;
	RefreshSlateContent();
}

void UDreamSlateHostWidget::RefreshSlateContent()
{
	if (!MyContainer.IsValid())
	{
		return;
	}

	MyContainer->SetContent(BuildHostedWidget().ToSharedRef());
	InvalidateLayoutAndVolatility();
}

void UDreamSlateHostWidget::PostLoad()
{
	Super::PostLoad();
	RefreshSlateContent();
}

void UDreamSlateHostWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RefreshSlateContent();
}

void UDreamSlateHostWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyContainer.Reset();
}

#if WITH_EDITOR
const FText UDreamSlateHostWidget::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "DreamUMG");
}

void UDreamSlateHostWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshSlateContent();
}
#endif

TSharedRef<SWidget> UDreamSlateHostWidget::RebuildWidget()
{
	SAssignNew(MyContainer, SBox);
	RefreshSlateContent();
	return MyContainer.ToSharedRef();
}

TSharedPtr<SWidget> UDreamSlateHostWidget::BuildHostedWidget() const
{
	if (RootWidget != nullptr)
	{
		return RootWidget->BuildWidget();
	}

	return StaticCastSharedRef<SWidget>(SNullWidget::NullWidget);
}

#undef LOCTEXT_NAMESPACE
