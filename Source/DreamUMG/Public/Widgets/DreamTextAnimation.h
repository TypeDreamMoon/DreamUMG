// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Widgets/DreamAnimatedTextTypes.h"
#include "DreamTextAnimation.generated.h"

struct FDreamTextAnimationLayoutContext
{
	int32 TotalAnimatedCharacters = 0;
	int32 TotalVisibleCharacters = 0;
};

struct FDreamTextAnimationGlyphContext
{
	int32 AnimatedIndex = INDEX_NONE;
	int32 TotalAnimatedCharacters = 0;
	int32 RevealOrdinal = 0;
	int32 TotalVisibleCharacters = 0;
};

struct FDreamTextAnimationSelectionResult
{
	bool bSelected = false;
	int32 SelectionStartIndex = 0;
	int32 SelectionEndIndex = 0;
	int32 SelectedCount = 0;
	int32 LocalIndex = 0;
	float RawValue = 0.0f;
	float ClampedValue = 0.0f;
};

struct FDreamTextAnimationGlyphState
{
	FVector2f Position = FVector2f::ZeroVector;
	FVector2f Scale = FVector2f(1.0f, 1.0f);
	float Rotation = 0.0f;
	float Opacity = 1.0f;
	FLinearColor Color = FLinearColor::White;
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamTextAnimationPlayerBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ResetPlayerState();

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool NeedsTick() const;

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PlayPlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PausePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void ResumePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void StopPlayback();

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPaused() const;

	virtual void ResetRuntimeState();
	virtual bool RequiresTick() const;
	virtual bool TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext);
	virtual int32 GetVisibleCharacterCount(int32 TotalVisibleCharacters) const;
	virtual void ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationGlyphState& InOutGlyphState) const;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Typewriter Player"))
class DREAMUMG_API UDreamTextTypewriterPlayer : public UDreamTextAnimationPlayerBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Offset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Playback")
	bool bAutoPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Playback")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Playback")
	bool bPingPong = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Playback")
	bool bPlayReverse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Playback", meta = (ClampMin = "0.01"))
	float Speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Reveal", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Start = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Reveal", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float End = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetOffset(float InOffset);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	float GetOffset() const { return Offset; }

	virtual void PlayPlayback() override;
	virtual void PausePlayback() override;
	virtual void ResumePlayback() override;
	virtual void StopPlayback() override;
	virtual bool IsPlaying() const override;
	virtual bool IsPaused() const override;

	virtual void ResetRuntimeState() override;
	virtual bool RequiresTick() const override;
	virtual bool TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext) override;
	virtual int32 GetVisibleCharacterCount(int32 TotalVisibleCharacters) const override;

private:
	float GetEffectiveOffset() const;

private:
	UPROPERTY(Transient)
	bool bManualPlayback = false;

	UPROPERTY(Transient)
	bool bPlaybackPaused = false;

	UPROPERTY(Transient)
	float RuntimeOffset = 0.0f;

	UPROPERTY(Transient)
	float RuntimeDirection = 1.0f;

	UPROPERTY(Transient)
	bool bRuntimeFinished = false;
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamTextAnimationSelectorBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Offset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Playback")
	bool bAutoPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Playback")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Playback")
	bool bPingPong = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Playback")
	bool bPlayReverse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Playback", meta = (ClampMin = "0.01"))
	float Speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Common", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Start = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Common", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float End = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetOffset(float InOffset);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	float GetOffset() const { return Offset; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ResetSelectorState();

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool NeedsTick() const;

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PlayPlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PausePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void ResumePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void StopPlayback();

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPaused() const;

	virtual void ResetRuntimeState();
	virtual bool RequiresTick() const;
	virtual bool TickSelector(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext);
	bool EvaluateSelection(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationSelectionResult& OutSelectionResult) const;

protected:
	virtual bool EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const PURE_VIRTUAL(UDreamTextAnimationSelectorBase::EvaluateSelectorValue, return false;);

	float GetEffectiveOffset() const;

protected:
	UPROPERTY(Transient)
	bool bManualPlayback = false;

	UPROPERTY(Transient)
	bool bPlaybackPaused = false;

	UPROPERTY(Transient)
	float RuntimeOffset = 0.0f;

	UPROPERTY(Transient)
	float RuntimeDirection = 1.0f;

	UPROPERTY(Transient)
	bool bRuntimeFinished = false;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Range Selector"))
class DREAMUMG_API UDreamTextAnimationRangeSelector : public UDreamTextAnimationSelectorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Range", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float Range = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Range")
	bool bFlipDirection = false;

protected:
	virtual bool EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Random Selector"))
class DREAMUMG_API UDreamTextAnimationRandomSelector : public UDreamTextAnimationSelectorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Random")
	int32 RandomSeed = 0;

protected:
	virtual bool EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Lyrics Selector"))
class DREAMUMG_API UDreamTextAnimationLyricsSelector : public UDreamTextAnimationSelectorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Lyrics", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float BlendRange = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selector|Lyrics")
	bool bFlipDirection = false;

protected:
	virtual bool EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const override;
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamTextAnimationExecutorBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor|Window", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Start = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor|Window", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float End = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ResetExecutorState();

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool NeedsTick() const;

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PlayPlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void PausePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void ResumePlayback();

	UFUNCTION(BlueprintCallable, Category = "Animation|Playback")
	virtual void StopPlayback();

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Animation|Playback")
	virtual bool IsPaused() const;

	virtual void ResetRuntimeState();
	virtual bool RequiresTick() const;
	virtual bool TickExecutor(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext);
	virtual void ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const;

protected:
	bool ResolveExecutorAlpha(const FDreamTextAnimationSelectionResult& SelectionResult, float& OutAlpha) const;
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamTextAnimationEaseExecutorBase : public UDreamTextAnimationExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	EDreamTextAnimationEase Ease = EDreamTextAnimationEase::InOutSine;

	virtual void ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const final;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const PURE_VIRTUAL(UDreamTextAnimationEaseExecutorBase::ApplyEasedGlyph, );
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class DREAMUMG_API UDreamTextAnimationWaveExecutorBase : public UDreamTextAnimationExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor", meta = (ClampMin = "0.01"))
	float Frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor", meta = (ClampMin = "0.01"))
	float Speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	bool bFlipDirection = false;

	virtual void PlayPlayback() override;
	virtual void PausePlayback() override;
	virtual void ResumePlayback() override;
	virtual void StopPlayback() override;
	virtual bool IsPlaying() const override;
	virtual bool IsPaused() const override;

	virtual void ResetRuntimeState() override;
	virtual bool RequiresTick() const override;
	virtual bool TickExecutor(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext) override;
	virtual void ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const final;

protected:
	virtual bool HasWaveEffect() const PURE_VIRTUAL(UDreamTextAnimationWaveExecutorBase::HasWaveEffect, return false;);
	virtual void ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const PURE_VIRTUAL(UDreamTextAnimationWaveExecutorBase::ApplyWaveGlyph, );

protected:
	UPROPERTY(Transient)
	bool bPlaybackPaused = false;

	UPROPERTY(Transient)
	float RuntimeTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Alpha Executor"))
class DREAMUMG_API UDreamTextAnimationAlphaExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Alpha = 0.0f;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Color Executor"))
class DREAMUMG_API UDreamTextAnimationColorExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	bool bUseHSV = true;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Color Random Executor"))
class DREAMUMG_API UDreamTextAnimationColorRandomExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FLinearColor Min = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FLinearColor Max = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	bool bUseHSV = true;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Position Executor"))
class DREAMUMG_API UDreamTextAnimationPositionExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Position = FVector2D::ZeroVector;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Position Random Executor"))
class DREAMUMG_API UDreamTextAnimationPositionRandomExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Min = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Max = FVector2D(0.0f, 10.0f);

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Position Wave Executor"))
class DREAMUMG_API UDreamTextAnimationPositionWaveExecutor : public UDreamTextAnimationWaveExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Position = FVector2D::ZeroVector;

protected:
	virtual bool HasWaveEffect() const override;
	virtual void ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Rotation Random Executor"))
class DREAMUMG_API UDreamTextAnimationRotationRandomExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	float Min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	float Max = 90.0f;

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Rotation Wave Executor"))
class DREAMUMG_API UDreamTextAnimationRotationWaveExecutor : public UDreamTextAnimationWaveExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	float Rotation = 0.0f;

protected:
	virtual bool HasWaveEffect() const override;
	virtual void ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Scale Executor"))
class DREAMUMG_API UDreamTextAnimationScaleExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Scale Random Executor"))
class DREAMUMG_API UDreamTextAnimationScaleRandomExecutor : public UDreamTextAnimationEaseExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Min = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Max = FVector2D(2.0f, 2.0f);

protected:
	virtual void ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Scale Wave Executor"))
class DREAMUMG_API UDreamTextAnimationScaleWaveExecutor : public UDreamTextAnimationWaveExecutorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Executor")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

protected:
	virtual bool HasWaveEffect() const override;
	virtual void ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, meta = (DisplayName = "Text Animation Player"))
class DREAMUMG_API UDreamTextAnimationPlayer : public UDreamTextAnimationPlayerBase
{
	GENERATED_BODY()

public:
	UDreamTextAnimationPlayer();

	virtual void PostLoad() override;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, BlueprintGetter = GetSelector, BlueprintSetter = SetSelector, Category = "Selector")
	TObjectPtr<UDreamTextAnimationSelectorBase> Selector;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Executors")
	TArray<TObjectPtr<UDreamTextAnimationExecutorBase>> Executors;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetOffset(float InOffset);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	float GetOffset() const { return Selector != nullptr ? Selector->GetOffset() : Offset; }

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Animation")
	void SetSelector(UDreamTextAnimationSelectorBase* InSelector);

	UFUNCTION(BlueprintPure, BlueprintGetter, Category = "Animation")
	UDreamTextAnimationSelectorBase* GetSelector() const { return Selector; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetExecutors(const TArray<UDreamTextAnimationExecutorBase*>& InExecutors);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void AddExecutor(UDreamTextAnimationExecutorBase* InExecutor);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool RemoveExecutor(UDreamTextAnimationExecutorBase* InExecutor);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ClearExecutors();

	UFUNCTION(BlueprintPure, Category = "Animation", meta = (DisplayName = "Get Executors"))
	TArray<UDreamTextAnimationExecutorBase*> GetExecutorsCopy() const;

	virtual void PlayPlayback() override;
	virtual void PausePlayback() override;
	virtual void ResumePlayback() override;
	virtual void StopPlayback() override;
	virtual bool IsPlaying() const override;
	virtual bool IsPaused() const override;

	virtual void ResetRuntimeState() override;
	virtual bool RequiresTick() const override;
	virtual bool TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext) override;
	virtual int32 GetVisibleCharacterCount(int32 TotalVisibleCharacters) const override;
	virtual void ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationGlyphState& InOutGlyphState) const override;
	bool EvaluateSelection(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationSelectionResult& OutSelectionResult) const;

	const TArray<TObjectPtr<UDreamTextAnimationExecutorBase>>& GetExecutors() const { return Executors; }

private:
	void EnsureSelector();
	void MigrateLegacySelectorData();

private:
	// Legacy serialized selector settings kept for backward compatibility with existing assets.
	UPROPERTY()
	EDreamTextAnimationPlayerMode SelectorMode = EDreamTextAnimationPlayerMode::Range;

	UPROPERTY()
	float Offset = 0.0f;

	UPROPERTY()
	bool bAutoPlay = false;

	UPROPERTY()
	bool bLoop = false;

	UPROPERTY()
	bool bPingPong = false;

	UPROPERTY()
	bool bPlayReverse = false;

	UPROPERTY()
	float Speed = 1.0f;

	UPROPERTY()
	float Range = 0.1f;

	UPROPERTY()
	bool bFlipDirection = false;

	UPROPERTY()
	float Start = 0.0f;

	UPROPERTY()
	float End = 1.0f;

	UPROPERTY()
	int32 RandomSeed = 0;
};
