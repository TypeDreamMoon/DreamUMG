// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamUMGBlueprintLibrary.generated.h"

class UDreamSlate3DWidget;
class UDreamTextAnimationExecutorBase;
class UDreamTextAnimationPlayerBase;
class UDreamTextAnimationSelectorBase;

UCLASS()
class DREAMUMG_API UDreamUMGBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DreamUMG", meta = (DefaultToSelf = "Outer", DeterminesOutputType = "PlayerClass", DynamicOutputParam = "ReturnValue"))
	static UDreamTextAnimationPlayerBase* CreateDreamTextAnimationPlayer(UObject* Outer, TSubclassOf<UDreamTextAnimationPlayerBase> PlayerClass);

	UFUNCTION(BlueprintCallable, Category = "DreamUMG", meta = (DefaultToSelf = "Outer", DeterminesOutputType = "SelectorClass", DynamicOutputParam = "ReturnValue"))
	static UDreamTextAnimationSelectorBase* CreateDreamTextAnimationSelector(UObject* Outer, TSubclassOf<UDreamTextAnimationSelectorBase> SelectorClass);

	UFUNCTION(BlueprintCallable, Category = "DreamUMG", meta = (DefaultToSelf = "Outer", DeterminesOutputType = "ExecutorClass", DynamicOutputParam = "ReturnValue"))
	static UDreamTextAnimationExecutorBase* CreateDreamTextAnimationExecutor(UObject* Outer, TSubclassOf<UDreamTextAnimationExecutorBase> ExecutorClass);

	UFUNCTION(BlueprintCallable, Category = "DreamUMG", meta = (DefaultToSelf = "Outer", DeterminesOutputType = "WidgetClass", DynamicOutputParam = "ReturnValue"))
	static UDreamSlate3DWidget* CreateDreamSlate3DWidget(UObject* Outer, TSubclassOf<UDreamSlate3DWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "DreamUMG")
	static void RefreshDreamSlate3DWidget(UDreamSlate3DWidget* Widget);
};
