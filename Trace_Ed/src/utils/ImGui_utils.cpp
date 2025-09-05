
#include "ImGui_utils.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "core/Utils.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

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

bool IsDockspaceFocused(ImGuiID dockspace_id)
{
	ImGuiContext* context = ImGui::GetCurrentContext();
	ImGuiWindow* curr_win = context->NavWindow;
	if (!curr_win)
	{
		return false;
	}

	if (curr_win->DockId == 0)
	{
		return false;
	}

	ImGuiDockNode* node = curr_win->DockNode;
	while (node)
	{
		if (node->ID == dockspace_id)
		{
			return true;
		}
		node = node->ParentNode;
	}

	return false;

}

void DrawGizmo(int mode, trace::Scene* scene, trace::UUID entity_id, trace::Camera* camera)
{
	trace::Entity selected_entity = scene->GetEntity(entity_id);
	if (mode != -1 && selected_entity)
	{

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
		glm::mat4 cam_view = camera->GetViewMatrix();
		glm::mat4 proj = camera->GetProjectionMatix();

		//TODO: Fix added due to vulkan viewport
		proj[1][1] *= -1.0f;
		trace::HierachyComponent& hi = selected_entity.GetComponent<trace::HierachyComponent>();
		trace::TransformComponent& pose = selected_entity.GetComponent<trace::TransformComponent>();
		bool has_parent = hi.HasParent();

		trace::Scene* scene = selected_entity.GetScene();

		glm::mat4 transform = hi.transform;


		bool modified = ImGuizmo::Manipulate(
			glm::value_ptr(cam_view),
			glm::value_ptr(proj),
			(ImGuizmo::OPERATION)mode,
			ImGuizmo::MODE::LOCAL,
			glm::value_ptr(transform),
			nullptr,// TODO: Check Docs {deltaMatrix}
			false,// snap
			nullptr,// TODO: Check Docs {localBounds}
			false //bounds snap
		);

		if (modified)
		{
			glm::vec3 pos, scale;
			glm::vec3 rotation;
			glm::quat rot;

			if (has_parent)
			{
				trace::Entity parent = scene->GetEntity(hi.parent);
				trace::HierachyComponent& parent_hi = parent.GetComponent<trace::HierachyComponent>();
				glm::mat4 parent_transform = scene->GetEntityGlobalPose(parent).GetLocalMatrix();
				parent_transform = glm::inverse(parent_transform);

				transform = parent_transform * transform;

			}

			//glm::decompose(transform, scale, rot, pos, skew, persp);
			trace::DecomposeMatrix(transform, pos, rotation, scale);
			rot = glm::quat((rotation));


			switch (mode)
			{
			case ImGuizmo::OPERATION::TRANSLATE:
			{
				pose._transform.SetPosition(pos);
				break;
			}
			case ImGuizmo::OPERATION::ROTATE:
			{
				pose._transform.SetRotation(rot);
				break;
			}
			case ImGuizmo::OPERATION::SCALE:
			{
				pose._transform.SetScale(scale);
				break;
			}
			}

		}



	}
}

void DrawGrid(trace::CommandList& cmd_list, float cell_size, uint32_t num_lines, int32_t draw_index)
{
	float line_lenght = cell_size * (num_lines - 1);

	trace::Renderer* renderer = trace::Renderer::get_instance();

	//Horizontal
	for (uint32_t i = 0; i < num_lines; i++)
	{
		float line_offset = line_lenght - (cell_size * 2.0f * (float)i);
		glm::vec3 from(line_lenght, 0.0f, line_offset);
		glm::vec3 to(-line_lenght, 0.0f, line_offset);

		if (!(line_offset == 0.0f))
		{
			renderer->DrawDebugLine(cmd_list, from, to, TRC_COL32(225, 225, 225, 105), draw_index);
		}
	}

	//Vertical
	for (uint32_t i = 0; i < num_lines; i++)
	{
		float line_offset = line_lenght - (cell_size * 2.0f * (float)i);
		glm::vec3 from(line_offset, 0.0f, line_lenght);
		glm::vec3 to(line_offset, 0.0f, -line_lenght);

		if (!(line_offset == 0.0f))
		{
			renderer->DrawDebugLine(cmd_list, from, to, TRC_COL32(225, 225, 225, 105), draw_index);
		}
	}

	// Global X-Coordinate
	renderer->DrawDebugLine(cmd_list, glm::vec3(line_lenght * 10.0f, 0.0f, 0.0f), glm::vec3(-line_lenght * 10.0f, 0.0f, 0.0f), TRC_COL32(255, 55, 55, 255), draw_index);

	// Global Y-Coordinate
	renderer->DrawDebugLine(cmd_list, glm::vec3(0.0f, line_lenght * 10.0f, 0.0f), glm::vec3(0.0f, -line_lenght * 10.0f, 0.0f), TRC_COL32(55, 255, 55, 255), draw_index);

	// Global Z-Coordinate
	renderer->DrawDebugLine(cmd_list, glm::vec3(0.0f, 0.0f, line_lenght * 10.0f), glm::vec3(0.0f, 0.0f, -line_lenght * 10.0f), TRC_COL32(55, 55, 255, 255), draw_index);

}
