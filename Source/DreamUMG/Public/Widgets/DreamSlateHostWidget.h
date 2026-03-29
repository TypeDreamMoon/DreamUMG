// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/DreamSlate3DWidgets.h"
#include "DreamSlateHostWidget.generated.h"

class SBox;

UCLASS(meta = (DisplayName = "Dream Slate Host"))
class DREAMUMG_API UDreamSlateHostWidget : public UWidget
{
	GENERATED_BODY()

public:
	UDreamSlateHostWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Dream Slate")
	TObjectPtr<UDreamSlate3DWidget> RootWidget;

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	void SetRootWidget(UDreamSlate3DWidget* InRootWidget);

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	void RefreshSlateContent();

	virtual void PostLoad() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<SWidget> BuildHostedWidget() const;

	TSharedPtr<SBox> MyContainer;
};
