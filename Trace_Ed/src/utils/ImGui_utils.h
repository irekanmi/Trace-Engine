#pragma once

#include "resource/Ref.h"
#include "external_utils.h"
#include "../TraceEditor.h"
#include "../panels/ContentBrowser.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "glm/glm.hpp"


#define IMGUI_WIDGET_MODIFIED_IF(modified, func, placeholder)    \
	bool b_##placeholder = false;                                  \
	b_##placeholder = func;                                        \
	modified = modified || b_##placeholder;                        \
	if(b_##placeholder)

namespace trace {
	class AnimationPanel;
}

bool DrawVec3(const char* label, glm::vec3& data, float column_width = 100.0f);
bool DrawVec3( glm::vec3& data, const char* id,float column_width = 100.0f);
float GetLineHeight();
bool IsDockspaceFocused(ImGuiID dockspace_id);
void DrawGizmo(int mode, trace::Scene* scene, trace::UUID entity_id, trace::Camera* camera, trace::AnimationPanel* animation_panel = nullptr);
void DrawGrid(trace::CommandList& cmd_list, float cell_size, uint32_t num_lines, int32_t draw_index);

template<typename T>
Ref<T> ImGuiDragDropResource(const std::string & tag)
{
	Ref<T> result;
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(tag.c_str()))
		{
			trace::TraceEditor* editor = trace::TraceEditor::get_instance();
			trace::ContentBrowser* content_browser = editor->GetContentBrowser();
			static char buf[1024] = { 0 };
			memcpy_s(buf, 1024, payload->Data, payload->DataSize);
			std::filesystem::path p = buf;
			trace::UUID id = trace::GetUUIDFromName(p.filename().string());
			result = T::Deserialize(id);
		}
		ImGui::EndDragDropTarget();
	}
	return result;
}

template<typename T>
Ref<T> ImGuiDragDropResourceCustom(ImRect rect, ImGuiID id, const std::string& tag)
{
	Ref<T> result;
	if (ImGui::BeginDragDropTargetCustom(rect, id))
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(tag.c_str()))
		{
			trace::TraceEditor* editor = trace::TraceEditor::get_instance();
			trace::ContentBrowser* content_browser = editor->GetContentBrowser();
			static char buf[1024] = { 0 };
			memcpy_s(buf, 1024, payload->Data, payload->DataSize);
			std::filesystem::path p = buf;
			trace::UUID id = trace::GetUUIDFromName(p.filename().string());
			result = T::Deserialize(id);
		}
		ImGui::EndDragDropTarget();
	}
	return result;
}

Ref<trace::GTexture> ImGuiDragDropTexture();

inline ImVec2 operator+(ImVec2& a, ImVec2& b)
{
	return ImVec2(a.x + b.x, a.y + b.y);
}