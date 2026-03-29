// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/DreamTextAnimation.h"

namespace DreamTextAnimation
{
	static float EvaluateEase(const EDreamTextAnimationEase Ease, const float Alpha)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		switch (Ease)
		{
		case EDreamTextAnimationEase::InSine:
			return 1.0f - FMath::Cos((ClampedAlpha * PI) * 0.5f);
		case EDreamTextAnimationEase::OutSine:
			return FMath::Sin((ClampedAlpha * PI) * 0.5f);
		case EDreamTextAnimationEase::InOutSine:
			return -(FMath::Cos(PI * ClampedAlpha) - 1.0f) * 0.5f;
		case EDreamTextAnimationEase::InQuad:
			return ClampedAlpha * ClampedAlpha;
		case EDreamTextAnimationEase::OutQuad:
			return 1.0f - (1.0f - ClampedAlpha) * (1.0f - ClampedAlpha);
		case EDreamTextAnimationEase::InOutQuad:
			return ClampedAlpha < 0.5f ? 2.0f * ClampedAlpha * ClampedAlpha : 1.0f - FMath::Pow(-2.0f * ClampedAlpha + 2.0f, 2.0f) * 0.5f;
		case EDreamTextAnimationEase::InCubic:
			return ClampedAlpha * ClampedAlpha * ClampedAlpha;
		case EDreamTextAnimationEase::OutCubic:
			return 1.0f - FMath::Pow(1.0f - ClampedAlpha, 3.0f);
		case EDreamTextAnimationEase::InOutCubic:
			return ClampedAlpha < 0.5f ? 4.0f * ClampedAlpha * ClampedAlpha * ClampedAlpha : 1.0f - FMath::Pow(-2.0f * ClampedAlpha + 2.0f, 3.0f) * 0.5f;
		case EDreamTextAnimationEase::Linear:
		default:
			return ClampedAlpha;
		}
	}

	static FRandomStream MakeDeterministicRandomStream(const int32 Seed, const int32 Index, const int32 Salt = 0)
	{
		const uint32 Hash = HashCombineFast(GetTypeHash(Seed), HashCombineFast(GetTypeHash(Index), GetTypeHash(Salt)));
		return FRandomStream(static_cast<int32>(Hash));
	}

	static float GetDeterministicRandom01(const int32 Seed, const int32 Index, const int32 Salt = 0)
	{
		FRandomStream Stream = MakeDeterministicRandomStream(Seed, Index, Salt);
		return Stream.FRand();
	}

	static float GetDeterministicRandomRange(const int32 Seed, const int32 Index, const int32 Salt, const float Min, const float Max)
	{
		FRandomStream Stream = MakeDeterministicRandomStream(Seed, Index, Salt);
		return Stream.FRandRange(Min, Max);
	}

	static FVector2D GetDeterministicRandomVector2D(const int32 Seed, const int32 Index, const FVector2D& Min, const FVector2D& Max)
	{
		return FVector2D(
			GetDeterministicRandomRange(Seed, Index, 17, Min.X, Max.X),
			GetDeterministicRandomRange(Seed, Index, 29, Min.Y, Max.Y)
		);
	}

	static FLinearColor GetDeterministicRandomColor(const int32 Seed, const int32 Index, const FLinearColor& Min, const FLinearColor& Max)
	{
		return FLinearColor(
			GetDeterministicRandomRange(Seed, Index, 11, Min.R, Max.R),
			GetDeterministicRandomRange(Seed, Index, 13, Min.G, Max.G),
			GetDeterministicRandomRange(Seed, Index, 17, Min.B, Max.B),
			GetDeterministicRandomRange(Seed, Index, 19, Min.A, Max.A)
		);
	}

	static FLinearColor LerpColor(const FLinearColor& From, const FLinearColor& To, const float Alpha, const bool bUseHSV)
	{
		return bUseHSV
			? FLinearColor::LerpUsingHSV(From, To, Alpha)
			: FMath::Lerp(From, To, Alpha);
	}

	static bool NearlyEqual(const float A, const float B)
	{
		return FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
	}

	static void ResetPlaybackState(const float Offset, const bool bPlayReverse, float& InOutRuntimeOffset, float& InOutRuntimeDirection, bool& InOutRuntimeFinished)
	{
		InOutRuntimeOffset = FMath::Clamp(Offset, 0.0f, 1.0f);
		InOutRuntimeDirection = bPlayReverse ? -1.0f : 1.0f;
		InOutRuntimeFinished = false;
	}

	static bool RequiresPlaybackTick(const bool bAutoPlay, const bool bRuntimeFinished, const bool bLoop, const bool bPingPong)
	{
		return bAutoPlay && (!bRuntimeFinished || bLoop || bPingPong);
	}

	static bool TickPlayback(const float DeltaTime, const bool bAutoPlay, const bool bLoop, const bool bPingPong, const float Speed, float& InOutRuntimeOffset, float& InOutRuntimeDirection, bool& InOutRuntimeFinished)
	{
		bool bKeepTicking = false;
		if (!bAutoPlay)
		{
			return false;
		}

		InOutRuntimeOffset += DeltaTime * Speed * InOutRuntimeDirection;

		if (bPingPong)
		{
			if (InOutRuntimeOffset >= 1.0f)
			{
				InOutRuntimeOffset = 1.0f;
				InOutRuntimeDirection = -1.0f;
			}
			else if (InOutRuntimeOffset <= 0.0f)
			{
				InOutRuntimeOffset = 0.0f;
				InOutRuntimeDirection = 1.0f;
			}

			bKeepTicking = true;
		}
		else if (bLoop)
		{
			if (InOutRuntimeOffset > 1.0f)
			{
				InOutRuntimeOffset = FMath::Fmod(InOutRuntimeOffset, 1.0f);
			}
			else if (InOutRuntimeOffset < 0.0f)
			{
				InOutRuntimeOffset = 1.0f + FMath::Fmod(InOutRuntimeOffset, 1.0f);
			}

			bKeepTicking = true;
		}
		else
		{
			InOutRuntimeOffset = FMath::Clamp(InOutRuntimeOffset, 0.0f, 1.0f);
			InOutRuntimeFinished = InOutRuntimeOffset <= 0.0f || InOutRuntimeOffset >= 1.0f;
			bKeepTicking = !InOutRuntimeFinished;
		}

		return bKeepTicking;
	}

	static bool IsSelectorAtDefaults(const UDreamTextAnimationSelectorBase* Selector)
	{
		if (Selector == nullptr)
		{
			return true;
		}

		const UDreamTextAnimationSelectorBase* DefaultSelector = Selector->GetClass()->GetDefaultObject<UDreamTextAnimationSelectorBase>();
		if (DefaultSelector == nullptr)
		{
			return false;
		}

		if (!NearlyEqual(Selector->Offset, DefaultSelector->Offset)
			|| Selector->bAutoPlay != DefaultSelector->bAutoPlay
			|| Selector->bLoop != DefaultSelector->bLoop
			|| Selector->bPingPong != DefaultSelector->bPingPong
			|| Selector->bPlayReverse != DefaultSelector->bPlayReverse
			|| !NearlyEqual(Selector->Speed, DefaultSelector->Speed)
			|| !NearlyEqual(Selector->Start, DefaultSelector->Start)
			|| !NearlyEqual(Selector->End, DefaultSelector->End))
		{
			return false;
		}

		if (const UDreamTextAnimationRangeSelector* RangeSelector = Cast<UDreamTextAnimationRangeSelector>(Selector))
		{
			const UDreamTextAnimationRangeSelector* DefaultRangeSelector = RangeSelector->GetClass()->GetDefaultObject<UDreamTextAnimationRangeSelector>();
			return DefaultRangeSelector != nullptr
				&& NearlyEqual(RangeSelector->Range, DefaultRangeSelector->Range)
				&& RangeSelector->bFlipDirection == DefaultRangeSelector->bFlipDirection;
		}

		if (const UDreamTextAnimationRandomSelector* RandomSelector = Cast<UDreamTextAnimationRandomSelector>(Selector))
		{
			const UDreamTextAnimationRandomSelector* DefaultRandomSelector = RandomSelector->GetClass()->GetDefaultObject<UDreamTextAnimationRandomSelector>();
			return DefaultRandomSelector != nullptr
				&& RandomSelector->RandomSeed == DefaultRandomSelector->RandomSeed;
		}

		if (const UDreamTextAnimationLyricsSelector* LyricsSelector = Cast<UDreamTextAnimationLyricsSelector>(Selector))
		{
			const UDreamTextAnimationLyricsSelector* DefaultLyricsSelector = LyricsSelector->GetClass()->GetDefaultObject<UDreamTextAnimationLyricsSelector>();
			return DefaultLyricsSelector != nullptr
				&& NearlyEqual(LyricsSelector->BlendRange, DefaultLyricsSelector->BlendRange)
				&& LyricsSelector->bFlipDirection == DefaultLyricsSelector->bFlipDirection;
		}

		return true;
	}
}

void UDreamTextAnimationPlayerBase::ResetRuntimeState()
{
}

bool UDreamTextAnimationPlayerBase::RequiresTick() const
{
	return false;
}

bool UDreamTextAnimationPlayerBase::TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	return false;
}

int32 UDreamTextAnimationPlayerBase::GetVisibleCharacterCount(int32 TotalVisibleCharacters) const
{
	return TotalVisibleCharacters;
}

void UDreamTextAnimationPlayerBase::ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
}

void UDreamTextTypewriterPlayer::SetOffset(float InOffset)
{
	Offset = FMath::Clamp(InOffset, 0.0f, 1.0f);
	RuntimeOffset = Offset;
	bRuntimeFinished = false;
}

void UDreamTextTypewriterPlayer::ResetRuntimeState()
{
	DreamTextAnimation::ResetPlaybackState(Offset, bPlayReverse, RuntimeOffset, RuntimeDirection, bRuntimeFinished);
}

bool UDreamTextTypewriterPlayer::RequiresTick() const
{
	return DreamTextAnimation::RequiresPlaybackTick(bAutoPlay, bRuntimeFinished, bLoop, bPingPong);
}

bool UDreamTextTypewriterPlayer::TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	return DreamTextAnimation::TickPlayback(DeltaTime, bAutoPlay, bLoop, bPingPong, Speed, RuntimeOffset, RuntimeDirection, bRuntimeFinished);
}

int32 UDreamTextTypewriterPlayer::GetVisibleCharacterCount(int32 TotalVisibleCharacters) const
{
	if (TotalVisibleCharacters <= 0 || End <= Start)
	{
		return 0;
	}

	const int32 RevealStartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(TotalVisibleCharacters) * Start), 0, TotalVisibleCharacters);
	const int32 RevealEndIndex = FMath::Clamp(FMath::CeilToInt(static_cast<float>(TotalVisibleCharacters) * End), 0, TotalVisibleCharacters);
	const int32 RevealSpan = FMath::Max(RevealEndIndex - RevealStartIndex, 0);
	if (RevealSpan <= 0)
	{
		return RevealStartIndex;
	}

	const float EffectiveOffset = FMath::Clamp(GetEffectiveOffset(), 0.0f, 1.0f);
	const int32 RevealedCount = EffectiveOffset <= 0.0f
		? 0
		: FMath::Clamp(FMath::CeilToInt(EffectiveOffset * static_cast<float>(RevealSpan)), 0, RevealSpan);
	return FMath::Clamp(RevealStartIndex + RevealedCount, 0, TotalVisibleCharacters);
}

float UDreamTextTypewriterPlayer::GetEffectiveOffset() const
{
	return bAutoPlay ? RuntimeOffset : Offset;
}

void UDreamTextAnimationSelectorBase::SetOffset(float InOffset)
{
	Offset = FMath::Clamp(InOffset, 0.0f, 1.0f);
	RuntimeOffset = Offset;
	bRuntimeFinished = false;
}

void UDreamTextAnimationSelectorBase::ResetRuntimeState()
{
	DreamTextAnimation::ResetPlaybackState(Offset, bPlayReverse, RuntimeOffset, RuntimeDirection, bRuntimeFinished);
}

bool UDreamTextAnimationSelectorBase::RequiresTick() const
{
	return DreamTextAnimation::RequiresPlaybackTick(bAutoPlay, bRuntimeFinished, bLoop, bPingPong);
}

bool UDreamTextAnimationSelectorBase::TickSelector(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	return DreamTextAnimation::TickPlayback(DeltaTime, bAutoPlay, bLoop, bPingPong, Speed, RuntimeOffset, RuntimeDirection, bRuntimeFinished);
}

bool UDreamTextAnimationSelectorBase::EvaluateSelection(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationSelectionResult& OutSelectionResult) const
{
	OutSelectionResult = FDreamTextAnimationSelectionResult();

	if (GlyphContext.AnimatedIndex == INDEX_NONE || GlyphContext.TotalAnimatedCharacters <= 0 || End <= Start)
	{
		return false;
	}

	const int32 SelectionStartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(GlyphContext.TotalAnimatedCharacters) * Start), 0, GlyphContext.TotalAnimatedCharacters);
	const int32 SelectionEndIndex = FMath::Clamp(FMath::CeilToInt(static_cast<float>(GlyphContext.TotalAnimatedCharacters) * End), 0, GlyphContext.TotalAnimatedCharacters);
	if (GlyphContext.AnimatedIndex < SelectionStartIndex || GlyphContext.AnimatedIndex >= SelectionEndIndex)
	{
		return false;
	}

	OutSelectionResult.bSelected = true;
	OutSelectionResult.SelectionStartIndex = SelectionStartIndex;
	OutSelectionResult.SelectionEndIndex = SelectionEndIndex;
	OutSelectionResult.SelectedCount = FMath::Max(SelectionEndIndex - SelectionStartIndex, 1);
	OutSelectionResult.LocalIndex = GlyphContext.AnimatedIndex - SelectionStartIndex;

	const float EffectiveOffset = FMath::Clamp(GetEffectiveOffset(), 0.0f, 1.0f);
	if (!EvaluateSelectorValue(GlyphContext, OutSelectionResult, EffectiveOffset, OutSelectionResult.RawValue))
	{
		OutSelectionResult = FDreamTextAnimationSelectionResult();
		return false;
	}

	OutSelectionResult.ClampedValue = FMath::Clamp(OutSelectionResult.RawValue, 0.0f, 1.0f);
	return true;
}

float UDreamTextAnimationSelectorBase::GetEffectiveOffset() const
{
	return bAutoPlay ? RuntimeOffset : Offset;
}

bool UDreamTextAnimationRangeSelector::EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const
{
	if (Range <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const int32 WeightedIndex = bFlipDirection ? (SelectionResult.SelectedCount - 1 - SelectionResult.LocalIndex) : SelectionResult.LocalIndex;
	const float Interval = 1.0f / static_cast<float>(SelectionResult.SelectedCount);
	const float CalculatedOffset = EffectiveOffset * (1.0f + Range) - Range;
	const float Value = (-CalculatedOffset) + (static_cast<float>(WeightedIndex) * Interval);
	OutRawValue = 1.0f - (Value / Range);
	return true;
}

bool UDreamTextAnimationRandomSelector::EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const
{
	const float CalculatedOffset = EffectiveOffset * 2.0f - 1.0f;
	OutRawValue = DreamTextAnimation::GetDeterministicRandom01(RandomSeed, SelectionResult.LocalIndex, SelectionResult.SelectedCount) + CalculatedOffset;
	return true;
}

bool UDreamTextAnimationLyricsSelector::EvaluateSelectorValue(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, float EffectiveOffset, float& OutRawValue) const
{
	if (SelectionResult.SelectedCount <= 0)
	{
		return false;
	}

	const int32 WeightedIndex = bFlipDirection ? (SelectionResult.SelectedCount - 1 - SelectionResult.LocalIndex) : SelectionResult.LocalIndex;
	const float CharacterPosition = static_cast<float>(WeightedIndex) / static_cast<float>(SelectionResult.SelectedCount);
	const float EffectiveBlendRange = FMath::Max(BlendRange, KINDA_SMALL_NUMBER);
	const float BlendEnd = FMath::Min(CharacterPosition + EffectiveBlendRange, 1.0f);

	if (BlendEnd <= CharacterPosition + KINDA_SMALL_NUMBER)
	{
		OutRawValue = EffectiveOffset >= CharacterPosition ? 1.0f : 0.0f;
		return true;
	}

	OutRawValue = FMath::GetMappedRangeValueClamped(FVector2f(CharacterPosition, BlendEnd), FVector2f(0.0f, 1.0f), EffectiveOffset);
	return true;
}

void UDreamTextAnimationExecutorBase::ResetRuntimeState()
{
}

bool UDreamTextAnimationExecutorBase::RequiresTick() const
{
	return false;
}

bool UDreamTextAnimationExecutorBase::TickExecutor(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	return false;
}

void UDreamTextAnimationExecutorBase::ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
}

bool UDreamTextAnimationExecutorBase::ResolveExecutorAlpha(const FDreamTextAnimationSelectionResult& SelectionResult, float& OutAlpha) const
{
	OutAlpha = 0.0f;

	if (!SelectionResult.bSelected || FMath::IsNearlyEqual(Start, End))
	{
		return false;
	}

	OutAlpha = FMath::Clamp(FMath::GetRangePct(Start, End, SelectionResult.ClampedValue), 0.0f, 1.0f);
	return OutAlpha > 0.0f;
}

void UDreamTextAnimationEaseExecutorBase::ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	float WindowAlpha = 0.0f;
	if (!ResolveExecutorAlpha(SelectionResult, WindowAlpha))
	{
		return;
	}

	const float Weight = DreamTextAnimation::EvaluateEase(Ease, WindowAlpha);
	if (Weight <= 0.0f)
	{
		return;
	}

	ApplyEasedGlyph(GlyphContext, Weight, InOutGlyphState);
}

void UDreamTextAnimationWaveExecutorBase::ResetRuntimeState()
{
	RuntimeTime = 0.0f;
}

bool UDreamTextAnimationWaveExecutorBase::RequiresTick() const
{
	return HasWaveEffect();
}

bool UDreamTextAnimationWaveExecutorBase::TickExecutor(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	RuntimeTime += DeltaTime;
	return HasWaveEffect();
}

void UDreamTextAnimationWaveExecutorBase::ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, const FDreamTextAnimationSelectionResult& SelectionResult, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	if (GlyphContext.AnimatedIndex == INDEX_NONE || !HasWaveEffect())
	{
		return;
	}

	float Weight = 0.0f;
	if (!ResolveExecutorAlpha(SelectionResult, Weight))
	{
		return;
	}

	const float Direction = bFlipDirection ? -1.0f : 1.0f;
	const float WaveValue = FMath::Sin((RuntimeTime * PI * Speed * Direction) + (static_cast<float>(GlyphContext.AnimatedIndex) * Frequency));
	ApplyWaveGlyph(GlyphContext, Weight, WaveValue, InOutGlyphState);
}

void UDreamTextAnimationAlphaExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Opacity *= FMath::Lerp(1.0f, Alpha, Weight);
}

void UDreamTextAnimationColorExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Color = DreamTextAnimation::LerpColor(InOutGlyphState.Color, Color, Weight, bUseHSV);
}

void UDreamTextAnimationColorRandomExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	const FLinearColor RandomColor = DreamTextAnimation::GetDeterministicRandomColor(Seed, GlyphContext.AnimatedIndex, Min, Max);
	InOutGlyphState.Color = DreamTextAnimation::LerpColor(InOutGlyphState.Color, RandomColor, Weight, bUseHSV);
}

void UDreamTextAnimationPositionExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Position += FVector2f(Position * Weight);
}

void UDreamTextAnimationPositionRandomExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	const FVector2D RandomPosition = DreamTextAnimation::GetDeterministicRandomVector2D(Seed, GlyphContext.AnimatedIndex, Min, Max);
	InOutGlyphState.Position += FVector2f(RandomPosition * Weight);
}

bool UDreamTextAnimationPositionWaveExecutor::HasWaveEffect() const
{
	return !Position.IsNearlyZero();
}

void UDreamTextAnimationPositionWaveExecutor::ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Position += FVector2f(Position * (WaveValue * Weight));
}

void UDreamTextAnimationRotationRandomExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	const float RandomRotation = DreamTextAnimation::GetDeterministicRandomRange(Seed, GlyphContext.AnimatedIndex, 31, Min, Max);
	InOutGlyphState.Rotation += RandomRotation * Weight;
}

bool UDreamTextAnimationRotationWaveExecutor::HasWaveEffect() const
{
	return !FMath::IsNearlyZero(Rotation);
}

void UDreamTextAnimationRotationWaveExecutor::ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Rotation += Rotation * (WaveValue * Weight);
}

void UDreamTextAnimationScaleExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	InOutGlyphState.Scale *= FMath::Lerp(FVector2f(1.0f, 1.0f), FVector2f(Scale), Weight);
}

void UDreamTextAnimationScaleRandomExecutor::ApplyEasedGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	const FVector2D RandomScale = DreamTextAnimation::GetDeterministicRandomVector2D(Seed, GlyphContext.AnimatedIndex, Min, Max);
	InOutGlyphState.Scale *= FMath::Lerp(FVector2f(1.0f, 1.0f), FVector2f(RandomScale), Weight);
}

bool UDreamTextAnimationScaleWaveExecutor::HasWaveEffect() const
{
	return !Scale.Equals(FVector2D(1.0f, 1.0f));
}

void UDreamTextAnimationScaleWaveExecutor::ApplyWaveGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, float Weight, float WaveValue, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	const FVector2f WaveScale = FVector2f(1.0f, 1.0f) + ((FVector2f(Scale) - FVector2f(1.0f, 1.0f)) * WaveValue * Weight);
	InOutGlyphState.Scale *= WaveScale;
}

UDreamTextAnimationPlayer::UDreamTextAnimationPlayer()
{
	Selector = CreateDefaultSubobject<UDreamTextAnimationRangeSelector>(TEXT("Selector"));
}

void UDreamTextAnimationPlayer::PostLoad()
{
	Super::PostLoad();
	MigrateLegacySelectorData();
	EnsureSelector();
	ResetRuntimeState();
}

void UDreamTextAnimationPlayer::SetOffset(float InOffset)
{
	EnsureSelector();
	if (Selector != nullptr)
	{
		Selector->SetOffset(InOffset);
	}
}

void UDreamTextAnimationPlayer::SetExecutors(const TArray<UDreamTextAnimationExecutorBase*>& InExecutors)
{
	Executors.Reset();
	Executors.Reserve(InExecutors.Num());

	for (UDreamTextAnimationExecutorBase* Executor : InExecutors)
	{
		if (Executor != nullptr)
		{
			Executors.Add(Executor);
		}
	}

	ResetRuntimeState();
}

void UDreamTextAnimationPlayer::ResetRuntimeState()
{
	EnsureSelector();
	if (Selector != nullptr)
	{
		Selector->ResetRuntimeState();
	}

	for (UDreamTextAnimationExecutorBase* Executor : Executors)
	{
		if (Executor != nullptr)
		{
			Executor->ResetRuntimeState();
		}
	}
}

bool UDreamTextAnimationPlayer::RequiresTick() const
{
	if (Selector != nullptr && Selector->RequiresTick())
	{
		return true;
	}

	for (const UDreamTextAnimationExecutorBase* Executor : Executors)
	{
		if (Executor != nullptr && Executor->RequiresTick())
		{
			return true;
		}
	}

	return false;
}

bool UDreamTextAnimationPlayer::TickPlayer(float DeltaTime, const FDreamTextAnimationLayoutContext& LayoutContext)
{
	bool bKeepTicking = false;

	if (Selector != nullptr)
	{
		bKeepTicking |= Selector->TickSelector(DeltaTime, LayoutContext);
	}

	for (UDreamTextAnimationExecutorBase* Executor : Executors)
	{
		if (Executor != nullptr && Executor->RequiresTick())
		{
			bKeepTicking |= Executor->TickExecutor(DeltaTime, LayoutContext);
		}
	}

	return bKeepTicking;
}

int32 UDreamTextAnimationPlayer::GetVisibleCharacterCount(int32 TotalVisibleCharacters) const
{
	return TotalVisibleCharacters;
}

void UDreamTextAnimationPlayer::ApplyToGlyph(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationGlyphState& InOutGlyphState) const
{
	FDreamTextAnimationSelectionResult SelectionResult;
	if (!EvaluateSelection(GlyphContext, SelectionResult))
	{
		return;
	}

	for (const TObjectPtr<UDreamTextAnimationExecutorBase>& Executor : Executors)
	{
		if (Executor != nullptr)
		{
			Executor->ApplyToGlyph(GlyphContext, SelectionResult, InOutGlyphState);
		}
	}
}

bool UDreamTextAnimationPlayer::EvaluateSelection(const FDreamTextAnimationGlyphContext& GlyphContext, FDreamTextAnimationSelectionResult& OutSelectionResult) const
{
	return Selector != nullptr && Selector->EvaluateSelection(GlyphContext, OutSelectionResult);
}

void UDreamTextAnimationPlayer::EnsureSelector()
{
	if (Selector == nullptr)
	{
		Selector = NewObject<UDreamTextAnimationRangeSelector>(this, NAME_None, RF_Transactional);
	}
}

void UDreamTextAnimationPlayer::MigrateLegacySelectorData()
{
	const bool bHasLegacyOverrides = SelectorMode != EDreamTextAnimationPlayerMode::Range
		|| !DreamTextAnimation::NearlyEqual(Offset, 0.0f)
		|| bAutoPlay
		|| bLoop
		|| bPingPong
		|| bPlayReverse
		|| !DreamTextAnimation::NearlyEqual(Speed, 1.0f)
		|| !DreamTextAnimation::NearlyEqual(Range, 0.1f)
		|| bFlipDirection
		|| !DreamTextAnimation::NearlyEqual(Start, 0.0f)
		|| !DreamTextAnimation::NearlyEqual(End, 1.0f)
		|| RandomSeed != 0;

	if (!bHasLegacyOverrides || !DreamTextAnimation::IsSelectorAtDefaults(Selector))
	{
		return;
	}

	UDreamTextAnimationSelectorBase* MigratedSelector = nullptr;
	
	if (SelectorMode == EDreamTextAnimationPlayerMode::Random)
	{
		UDreamTextAnimationRandomSelector* RandomSelector = Cast<UDreamTextAnimationRandomSelector>(Selector);
		if (RandomSelector == nullptr)
		{
			RandomSelector = NewObject<UDreamTextAnimationRandomSelector>(this, NAME_None, RF_Transactional);
		}

		RandomSelector->RandomSeed = RandomSeed;
		MigratedSelector = RandomSelector;
	}
	else
	{
		UDreamTextAnimationRangeSelector* RangeSelector = Cast<UDreamTextAnimationRangeSelector>(Selector);
		if (RangeSelector == nullptr)
		{
			RangeSelector = NewObject<UDreamTextAnimationRangeSelector>(this, NAME_None, RF_Transactional);
		}

		RangeSelector->Range = Range;
		RangeSelector->bFlipDirection = bFlipDirection;
		MigratedSelector = RangeSelector;
	}

	if (MigratedSelector != nullptr)
	{
		MigratedSelector->Offset = FMath::Clamp(Offset, 0.0f, 1.0f);
		MigratedSelector->bAutoPlay = bAutoPlay;
		MigratedSelector->bLoop = bLoop;
		MigratedSelector->bPingPong = bPingPong;
		MigratedSelector->bPlayReverse = bPlayReverse;
		MigratedSelector->Speed = FMath::Max(Speed, 0.01f);
		MigratedSelector->Start = FMath::Clamp(Start, 0.0f, 1.0f);
		MigratedSelector->End = FMath::Clamp(End, 0.0f, 1.0f);
		Selector = MigratedSelector;
	}

	SelectorMode = EDreamTextAnimationPlayerMode::Range;
	Offset = 0.0f;
	bAutoPlay = false;
	bLoop = false;
	bPingPong = false;
	bPlayReverse = false;
	Speed = 1.0f;
	Range = 0.1f;
	bFlipDirection = false;
	Start = 0.0f;
	End = 1.0f;
	RandomSeed = 0;
}
