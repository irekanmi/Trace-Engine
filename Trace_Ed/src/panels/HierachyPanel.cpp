
#include "HierachyPanel.h"
#include "../TraceEditor.h"
#include "imgui.h"
#include "scene/Componets.h"

namespace trace {
	HierachyPanel::HierachyPanel()
	{
		
	}
	void HierachyPanel::Render(float deltaTime)
	{
		ImGui::Begin("Scene Hierachy", 0, ImGuiWindowFlags_NoCollapse);

		for (auto& [entity] : m_editor->m_currentScene->m_registry.storage<entt::entity>().each())
		{
			TagComponent& Tag = m_editor->m_currentScene->m_registry.get<TagComponent>(entity);
			ImGui::Text("%s", Tag.tag.c_str());
		}


		ImGui::End();

	}
}