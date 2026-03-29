// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DreamAnimatedTextTypes.generated.h"

UENUM(BlueprintType)
enum class EDreamTextAnimationEase : uint8
{
	Linear,
	InSine,
	OutSine,
	InOutSine,
	InQuad,
	OutQuad,
	InOutQuad,
	InCubic,
	OutCubic,
	InOutCubic
};

UENUM(BlueprintType)
enum class EDreamTextAnimationPlayerMode : uint8
{
	Range,
	Random
};
