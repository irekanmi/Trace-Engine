#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "glm/glm.hpp"
#include <string>

inline float lerp(float a, float b, float t)
{
	return a +((b - a) * t);
}

namespace trace {

	bool FindDirectory(const std::string& from, const std::string& dir, std::string& result);
	
	//TODO: Move function to a math module
	void DecomposeMatrix(glm::mat4 matrix, glm::vec3& pos, glm::vec3& rotation, glm::vec3& scale);

	std::vector<std::string> SplitString(const std::string& str, char token);

	glm::vec4 colorFromUint32(uint32_t color);
	uint32_t colorVec4ToUint(glm::vec4 color);

}

