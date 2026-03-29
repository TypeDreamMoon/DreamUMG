// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Widgets/DreamSlate3DWidgets.h"
#include "DreamWidgetComponent.generated.h"

class FPrimitiveSceneProxy;

UCLASS(ClassGroup = UI, BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName = "Dream Widget Component"))
class DREAMUMG_API UDreamWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDreamWidgetComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, BlueprintGetter = GetRootWidget, BlueprintSetter = SetRootWidget, Category = "Dream Widget")
	TObjectPtr<UDreamSlate3DWidget> RootWidget;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Dream Widget")
	void SetRootWidget(UDreamSlate3DWidget* InRootWidget);

	UFUNCTION(BlueprintPure, BlueprintGetter, Category = "Dream Widget")
	UDreamSlate3DWidget* GetRootWidget() const { return RootWidget; }

	UFUNCTION(BlueprintCallable, Category = "Dream Widget")
	void RebuildSlateWidget();

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	virtual void PostLoad() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SetWidget(UUserWidget* Widget) override;
	virtual void SetSlateWidget(const TSharedPtr<SWidget>& InSlateWidget) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual bool ShouldDrawWidget() const override;
	virtual void OnHiddenInGameChanged() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;

private:
	void UpdateRendererEntry();
	void RemoveRendererEntry();

	TSharedPtr<SWidget> BuiltSlateWidget;
};
