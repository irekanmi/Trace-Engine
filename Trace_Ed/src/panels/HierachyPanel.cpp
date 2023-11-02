
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
		ImGui::Begin("Scene Hierachy", 0, ImGuiWindowFlags_NoCollapse);

		if (m_editor->m_currentScene)
		{
			for (auto& [entity] : m_editor->m_currentScene->m_registry.storage<entt::entity>().each())
			{

				Entity current_entity(entity, m_editor->m_currentScene.get());
				DrawEntity(current_entity);

			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
				m_selectedEntity = Entity();

			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					m_editor->m_currentScene->CreateEntity();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();


	}
	void HierachyPanel::DrawEntity(Entity entity)
	{
		
		bool selected = (m_selectedEntity == entity);
		ImGuiTreeNodeFlags tree_flags = ( selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		TagComponent& tag = m_editor->m_currentScene->m_registry.get<TagComponent>(entity);
		void* id = (void*)(uint64_t)(uint32_t)entity;
		bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag._tag.c_str());

		if (ImGui::IsItemClicked())
		{
			if (selected)
				m_selectedEntity = Entity();
			else
				m_selectedEntity = entity;

			m_editor->m_inspectorPanel.SetDrawCallbackFn([&]()
				{
					if (m_selectedEntity)
						m_editor->m_inspectorPanel.DrawEntityComponent(m_selectedEntity);
				});
		}

		//FIX: Rendering twice if tag is the same
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) 
			{ 
				m_editor->m_currentScene->DestroyEntity(entity);
				if (selected) { m_selectedEntity = Entity(); }
			}
			ImGui::EndPopup();
		}


		if (clicked) ImGui::TreePop();


		
	}
}