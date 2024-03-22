
#include "ImGui_utils.h"
#include "imgui.h"
#include "glm/glm.hpp"
#include "imgui_internal.h"

bool DrawVec3(const char* label, glm::vec3& data, float column_width)
{
	bool modified = false;
	ImGui::PushID(label);

	ImGui::Columns(2);

	ImGui::SetColumnWidth(0, column_width);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.15f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.15f, 0.15f, 1.0f });
	ImGui::Button("X", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##X", &data.x, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.8f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.9f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.8f, 0.15f, 1.0f });
	ImGui::Button("Y", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Y", &data.y, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);


	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.15f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.2f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.15f, 0.8f, 1.0f });
	ImGui::Button("Z", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Z", &data.z, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);


	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();

	return modified;
}

bool DrawVec3(glm::vec3& data, const char* id, float column_width)
{
	bool modified = false;
	ImGui::PushID(id);

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.15f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.15f, 0.15f, 1.0f });
	ImGui::Button("X", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##X", &data.x, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.8f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.9f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.8f, 0.15f, 1.0f });
	ImGui::Button("Y", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Y", &data.y, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);


	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.15f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.2f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.15f, 0.8f, 1.0f });
	ImGui::Button("Z", button_size);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Z", &data.z, 0.1f)) modified = true;
	ImGui::PopItemWidth();
	ImGui::PopStyleColor(3);


	ImGui::PopStyleVar();
	ImGui::PopID();

	return modified;
}

float GetLineHeight()
{
	return GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
}
