
#include "InspectorPanel.h"
#include "scene/Componets.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "glm/gtc/type_ptr.hpp"

namespace trace {

	InspectorPanel::InspectorPanel()
	{
	}

	template<typename T, typename DrawFunc>
	static void DrawComponent(Entity entity, const char* placeholder, DrawFunc func)
	{
		if (entity.HasComponent<T>())
		{
			T& component = entity.GetComponent<T>();
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
			if (ImGui::TreeNodeEx(placeholder, tree_flags))
			{
				func(entity, component);
				ImGui::TreePop();
			}
		}
	}

	void InspectorPanel::DrawEntityComponent(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			TagComponent& val = entity.GetComponent<TagComponent>();
			std::string temp = val._tag;
			if (ImGui::InputText("Tag", &temp))
			{
				if (!temp.empty())
				{
					val._tag = std::move(temp);
				}

			}
		}

		DrawComponent<TransformComponent>(entity, "Transform", [](Entity obj, TransformComponent& comp) {

			glm::vec3 pos = comp._transform.GetPosition();
			glm::vec3 scale = comp._transform.GetScale();
			if (ImGui::DragFloat3("Position", glm::value_ptr(pos)))
				comp._transform.SetPosition(pos);
			if (ImGui::DragFloat3("Scale", glm::value_ptr(scale)))
				comp._transform.SetScale(scale);

			});
	}

}