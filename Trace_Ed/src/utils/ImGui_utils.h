#pragma once

#include "glm/glm.hpp"

bool DrawVec3(const char* label, glm::vec3& data, float column_width = 100.0f);
bool DrawVec3( glm::vec3& data, const char* id,float column_width = 100.0f);
float GetLineHeight();