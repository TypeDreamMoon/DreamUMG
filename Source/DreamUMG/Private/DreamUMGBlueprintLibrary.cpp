// Copyright Type Dream Moon All Rights Reserved.

#include "DreamUMGBlueprintLibrary.h"

#include "Widgets/DreamSlate3DWidgets.h"
#include "Widgets/DreamTextAnimation.h"

UDreamTextAnimationPlayerBase* UDreamUMGBlueprintLibrary::CreateDreamTextAnimationPlayer(UObject* Outer, TSubclassOf<UDreamTextAnimationPlayerBase> PlayerClass)
{
	UClass* ClassToCreate = PlayerClass != nullptr ? PlayerClass.Get() : UDreamTextAnimationPlayer::StaticClass();
	if (ClassToCreate == nullptr || ClassToCreate->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	return NewObject<UDreamTextAnimationPlayerBase>(Outer != nullptr ? Outer : GetTransientPackage(), ClassToCreate, NAME_None, RF_Transactional);
}

UDreamTextAnimationSelectorBase* UDreamUMGBlueprintLibrary::CreateDreamTextAnimationSelector(UObject* Outer, TSubclassOf<UDreamTextAnimationSelectorBase> SelectorClass)
{
	UClass* ClassToCreate = SelectorClass != nullptr ? SelectorClass.Get() : UDreamTextAnimationRangeSelector::StaticClass();
	if (ClassToCreate == nullptr || ClassToCreate->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	return NewObject<UDreamTextAnimationSelectorBase>(Outer != nullptr ? Outer : GetTransientPackage(), ClassToCreate, NAME_None, RF_Transactional);
}

UDreamTextAnimationExecutorBase* UDreamUMGBlueprintLibrary::CreateDreamTextAnimationExecutor(UObject* Outer, TSubclassOf<UDreamTextAnimationExecutorBase> ExecutorClass)
{
	UClass* ClassToCreate = ExecutorClass != nullptr ? ExecutorClass.Get() : UDreamTextAnimationAlphaExecutor::StaticClass();
	if (ClassToCreate == nullptr || ClassToCreate->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	return NewObject<UDreamTextAnimationExecutorBase>(Outer != nullptr ? Outer : GetTransientPackage(), ClassToCreate, NAME_None, RF_Transactional);
}

UDreamSlate3DWidget* UDreamUMGBlueprintLibrary::CreateDreamSlate3DWidget(UObject* Outer, TSubclassOf<UDreamSlate3DWidget> WidgetClass)
{
	if (WidgetClass == nullptr || WidgetClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	return NewObject<UDreamSlate3DWidget>(Outer != nullptr ? Outer : GetTransientPackage(), WidgetClass, NAME_None, RF_Transactional);
}

void UDreamUMGBlueprintLibrary::RefreshDreamSlate3DWidget(UDreamSlate3DWidget* Widget)
{
	if (Widget != nullptr)
	{
		Widget->RefreshOwner();
	}
}
