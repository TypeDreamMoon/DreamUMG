// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextWidgetTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "UObject/Object.h"
#include "DreamSlate3DWidgets.generated.h"

class SWidget;
class UDreamSlateWidgetComponent;
class UDreamWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDreamSlate3DButtonClicked);

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamSlate3DWidget : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	void RefreshOwner();

	virtual TSharedRef<SWidget> BuildWidget() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	static TSharedRef<SWidget> BuildChildWidget(const UDreamSlate3DWidget* Child);
	void NotifyOwnerChanged() const;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Slate 3D Text"))
class DREAMUMG_API UDreamSlate3DText : public UDreamSlate3DWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TEnumAsByte<ETextJustify::Type> Justification = ETextJustify::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	ETextTransformPolicy TransformPolicy = ETextTransformPolicy::None;

	virtual TSharedRef<SWidget> BuildWidget() const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Slate 3D Image"))
class DREAMUMG_API UDreamSlate3DImage : public UDreamSlate3DWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D DesiredSize = FVector2D::ZeroVector;

	virtual TSharedRef<SWidget> BuildWidget() const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Slate 3D Border"))
class DREAMUMG_API UDreamSlate3DBorder : public UDreamSlate3DWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, BlueprintGetter = GetContentWidget, BlueprintSetter = SetContentWidget, Category = "Content")
	TObjectPtr<UDreamSlate3DWidget> Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FMargin Padding = FMargin(8.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor BrushColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = VAlign_Fill;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Dream Slate")
	void SetContentWidget(UDreamSlate3DWidget* InContent);

	UFUNCTION(BlueprintPure, BlueprintGetter, Category = "Dream Slate")
	UDreamSlate3DWidget* GetContentWidget() const { return Content; }

	virtual TSharedRef<SWidget> BuildWidget() const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Slate 3D Button"))
class DREAMUMG_API UDreamSlate3DButton : public UDreamSlate3DWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, BlueprintGetter = GetContentWidget, BlueprintSetter = SetContentWidget, Category = "Content")
	TObjectPtr<UDreamSlate3DWidget> Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FText FallbackText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FSlateColor ForegroundColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ButtonColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FMargin ContentPadding = FMargin(12.0f, 8.0f);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDreamSlate3DButtonClicked OnClicked;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Dream Slate")
	void SetContentWidget(UDreamSlate3DWidget* InContent);

	UFUNCTION(BlueprintPure, BlueprintGetter, Category = "Dream Slate")
	UDreamSlate3DWidget* GetContentWidget() const { return Content; }

	virtual TSharedRef<SWidget> BuildWidget() const override;

private:
	FReply HandleClicked();
};

USTRUCT(BlueprintType)
struct DREAMUMG_API FDreamSlate3DVerticalBoxSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Content")
	TObjectPtr<UDreamSlate3DWidget> Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FMargin Padding = FMargin(0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = VAlign_Fill;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Slate 3D Vertical Box"))
class DREAMUMG_API UDreamSlate3DVerticalBox : public UDreamSlate3DWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FMargin SlotPadding = FMargin(0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FDreamSlate3DVerticalBoxSlot> Children;

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	int32 AddChildWidget(UDreamSlate3DWidget* InContent);

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	int32 AddChildSlot(const FDreamSlate3DVerticalBoxSlot& InSlot);

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	bool RemoveChildWidget(UDreamSlate3DWidget* InContent);

	UFUNCTION(BlueprintCallable, Category = "Dream Slate")
	void ClearChildren();

	UFUNCTION(BlueprintPure, Category = "Dream Slate")
	int32 GetChildrenCount() const { return Children.Num(); }

	virtual TSharedRef<SWidget> BuildWidget() const override;
};
