// Copyright Type Dream Moon All Rights Reserved.

#include "Widgets/DreamSlate3DWidgets.h"

#include "Components/DreamSlateWidgetComponent.h"
#include "Widgets/DreamSlateHostWidget.h"
#include "Input/Reply.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UDreamSlate3DWidget::BuildWidget() const
{
	return SNullWidget::NullWidget;
}

#if WITH_EDITOR
void UDreamSlate3DWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (UDreamSlateWidgetComponent* OwningComponent = FindOwningComponent())
	{
		OwningComponent->RebuildSlateWidget();
		return;
	}

	for (UObject* CurrentOuter = GetOuter(); CurrentOuter != nullptr; CurrentOuter = CurrentOuter->GetOuter())
	{
		if (UDreamSlateHostWidget* HostWidget = Cast<UDreamSlateHostWidget>(CurrentOuter))
		{
			HostWidget->RefreshSlateContent();
			return;
		}
	}
}
#endif

TSharedRef<SWidget> UDreamSlate3DWidget::BuildChildWidget(const UDreamSlate3DWidget* Child)
{
	return Child != nullptr ? Child->BuildWidget() : SNullWidget::NullWidget;
}

UDreamSlateWidgetComponent* UDreamSlate3DWidget::FindOwningComponent() const
{
	for (const UObject* CurrentOuter = GetOuter(); CurrentOuter != nullptr; CurrentOuter = CurrentOuter->GetOuter())
	{
		if (const UDreamSlateWidgetComponent* Component = Cast<UDreamSlateWidgetComponent>(CurrentOuter))
		{
			return const_cast<UDreamSlateWidgetComponent*>(Component);
		}
	}

	return nullptr;
}

TSharedRef<SWidget> UDreamSlate3DText::BuildWidget() const
{
	return SNew(STextBlock)
		.Text(Text)
		.ColorAndOpacity(ColorAndOpacity)
		.Font(Font)
		.Justification(Justification)
		.TransformPolicy(TransformPolicy);
}

TSharedRef<SWidget> UDreamSlate3DImage::BuildWidget() const
{
	const TSharedRef<SImage> ImageWidget = SNew(SImage)
		.Image(&Brush)
		.ColorAndOpacity(ColorAndOpacity);

	if (!DesiredSize.IsNearlyZero())
	{
		return SNew(SBox)
			.WidthOverride(DesiredSize.X)
			.HeightOverride(DesiredSize.Y)
			[
				ImageWidget
			];
	}

	return ImageWidget;
}

TSharedRef<SWidget> UDreamSlate3DBorder::BuildWidget() const
{
	return SNew(SBorder)
		.Padding(Padding)
		.HAlign(HorizontalAlignment)
		.VAlign(VerticalAlignment)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(BrushColor)
		[
			BuildChildWidget(Content)
		];
}

TSharedRef<SWidget> UDreamSlate3DButton::BuildWidget() const
{
	TSharedRef<SWidget> ButtonContent = Content != nullptr
		? Content->BuildWidget()
		: StaticCastSharedRef<SWidget>(
			SNew(STextBlock)
			.Text(FallbackText)
			.ColorAndOpacity(ForegroundColor));

	return SNew(SButton)
		.ButtonColorAndOpacity(ButtonColor)
		.ContentPadding(ContentPadding)
		.OnClicked(FOnClicked::CreateUObject(const_cast<UDreamSlate3DButton*>(this), &UDreamSlate3DButton::HandleClicked))
		[
			ButtonContent
		];
}

FReply UDreamSlate3DButton::HandleClicked()
{
	OnClicked.Broadcast();
	return FReply::Handled();
}

TSharedRef<SWidget> UDreamSlate3DVerticalBox::BuildWidget() const
{
	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	for (const FDreamSlate3DVerticalBoxSlot& Slot : Children)
	{
		VerticalBox->AddSlot()
			.Padding(Slot.Padding + SlotPadding)
			.HAlign(Slot.HorizontalAlignment)
			.VAlign(Slot.VerticalAlignment)
			[
				BuildChildWidget(Slot.Content)
			];
	}

	return VerticalBox;
}
