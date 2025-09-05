
#include "EditorWindow.h"
#include "../TraceEditor.h"
#include "../utils/ImGui_utils.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace trace {
	bool EditorWindow::OnCreate(TraceEditor* editor, const std::string& window_name)
	{
		m_name = window_name;

		return false;
	}
	void EditorWindow::OnDestroy(TraceEditor* editor)
	{
	}
	void EditorWindow::OnUpdate(float deltaTime)
	{
	}
	void EditorWindow::OnRender(float deltaTime)
	{
	}
	bool EditorWindow::CanReceiveEvent()
	{
		return m_isOpen && is_focused;
	}
	void EditorWindow::OnWindowOpen()
	{
	}
	void EditorWindow::OnWindowLeave()
	{
	}
	void EditorWindow::OnWindowClose()
	{
	}
	void EditorWindow::DockChildWindows()
	{
	}
	bool EditorWindow::Render(float deltaTime)
	{
		if (can_close && first_run)
		{
			TraceEditor::get_instance()->AddWindowToDockspace(m_name);
		}

		bool* close_ptr = can_close ? &m_windowOpen : nullptr;
		ImGuiWindowFlags window_flags = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		bool open = ImGui::Begin(m_name.c_str(), close_ptr, window_flags);

		if (open && !m_openLastFrame)
		{
			m_isOpen = true;
			OnWindowOpen();
		}

		dockspace_id = ImGui::GetID(m_name.c_str());
		ImGui::DockSpace(dockspace_id);

		if (first_run)
		{
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImVec2 size = ImGui::GetContentRegionAvail();
			if (size.x < 0.0f)
			{
				size.x = 5.0f;
			}
			if (size.y < 0.0f)
			{
				size.y = 5.0f;
			}
			ImGui::DockBuilderSetNodeSize(dockspace_id, size);

			DockChildWindows();

			ImGui::DockBuilderFinish(dockspace_id);

			first_run = false;
			
		}


		ImGui::End();
		ImGui::PopStyleVar(2);

		if (open)
		{
			OnRender(deltaTime);
		}
		else
		{
			if (m_openLastFrame)
			{
				m_isOpen = false;
				OnWindowLeave();
			}
		}
		is_focused = IsDockspaceFocused(dockspace_id);

		m_openLastFrame = open;
		return m_windowOpen;
	}
	void EditorWindow::RenderViewport(std::vector<void*>& texture_handles)
	{
	}
	void EditorWindow::OnEvent(Event* p_event)
	{
	}
}