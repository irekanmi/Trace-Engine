#pragma once

#include "glm/glm.hpp"

#define IMGUI_WIDGET_MODIFIED_IF(modified, func, placeholder)    \
	bool b_##placeholder = false;                                  \
	b_##placeholder = func;                                        \
	modified = modified || b_##placeholder;                        \
	if(b_##placeholder)

bool DrawVec3(const char* label, glm::vec3& data, float column_width = 100.0f);
bool DrawVec3( glm::vec3& data, const char* id,float column_width = 100.0f);
float GetLineHeight();