#pragma once

#include "resource/Ref.h"

#include "glm/glm.hpp"
#include <string>


#define IMGUI_WIDGET_MODIFIED_IF(modified, func, placeholder)    \
	bool b_##placeholder = false;                                  \
	b_##placeholder = func;                                        \
	modified = modified || b_##placeholder;                        \
	if(b_##placeholder)

bool DrawVec3(const char* label, glm::vec3& data, float column_width = 100.0f);
bool DrawVec3( glm::vec3& data, const char* id,float column_width = 100.0f);
float GetLineHeight();

template<typename T>
Ref<T> ImGuiDragDropResource(const std::string & tag)
{
	Ref<T> result;
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(tag.c_str()))
		{
			static char buf[1024] = { 0 };
			memcpy_s(buf, 1024, payload->Data, payload->DataSize);
			std::filesystem::path p = buf;
			result = T::Deserialize(p.string());
		}
		ImGui::EndDragDropTarget();
	}
	return result;
}