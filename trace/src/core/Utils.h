#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "core/Coretypes.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>



namespace trace {

	bool FindDirectory(const std::string& from, const std::string& dir, std::string& result);
	
	//TODO: Move function to a math module
	void DecomposeMatrix(glm::mat4 matrix, glm::vec3& pos, glm::vec3& rotation, glm::vec3& scale);

	std::vector<std::string> SplitString(const std::string& str, char token);

	glm::vec4 colorFromUint32(uint32_t color);
	uint32_t colorVec4ToUint(glm::vec4 color);

	StringID GetStringID(const std::string& data);

	std::string& GetStringFromID(StringID string_id);

	inline float lerp(float a, float b, float t)
	{
		return a + ((b - a) * t);
	}

	inline glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t)
	{
		return a + ((b - a) * t);
	}

	inline glm::quat slerp(glm::quat a, glm::quat b, float t)
	{
		return glm::slerp(a, b, t);
	}

	inline uint64_t hash_string(const std::string& str);

}

#define STR_ID(string) trace::GetStringID(string)
#define STRING_FROM_ID(string_id) trace::GetStringFromID(string_id)

