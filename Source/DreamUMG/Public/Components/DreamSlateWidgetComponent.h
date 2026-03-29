// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Widgets/DreamSlate3DWidgets.h"
#include "DreamSlateWidgetComponent.generated.h"

UCLASS(ClassGroup = UI, BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName = "Dream Slate Widget Component"))
class DREAMUMG_API UDreamSlateWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDreamSlateWidgetComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, BlueprintGetter = GetRootWidget, BlueprintSetter = SetRootWidget, Category = "Dream Slate 3D")
	TObjectPtr<UDreamSlate3DWidget> RootWidget;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Dream Slate 3D")
	void SetRootWidget(UDreamSlate3DWidget* InRootWidget);

	UFUNCTION(BlueprintPure, BlueprintGetter, Category = "Dream Slate 3D")
	UDreamSlate3DWidget* GetRootWidget() const { return RootWidget; }

	UFUNCTION(BlueprintCallable, Category = "Dream Slate 3D")
	void RebuildSlateWidget();

	virtual void OnRegister() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	TSharedPtr<SWidget> BuiltSlateWidget;
};
