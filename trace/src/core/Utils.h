#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include <string>

inline float lerp(float a, float b, float t)
{
	return a +((b - a) * t);
}

namespace trace {

	bool FindDirectory(const std::string& from, const std::string& dir, std::string& result);
}

