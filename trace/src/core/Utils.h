#pragma once

#include "core/Core.h"
#include "core/Enums.h"


inline float lerp(float a, float b, float t)
{
	return a +((b - a) * t);
}


