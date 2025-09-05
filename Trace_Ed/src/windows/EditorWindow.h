#pragma once

#include "core/events/Events.h"

#include <string>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h"

namespace trace {

	class TraceEditor;

	class EditorWindow
	{

	public:

		bool OnCreate(TraceEditor* editor, const std::string& window_name);
		virtual void OnDestroy(TraceEditor* editor);
		virtual void OnUpdate(float deltaTime);
		virtual void OnRender(float deltaTime);
		virtual bool CanReceiveEvent();
		virtual void OnWindowOpen();
		virtual void OnWindowLeave();
		virtual void OnWindowClose();
		virtual void DockChildWindows();
		std::string GetName() { return m_name; }
		bool Closed() { return !m_windowOpen; }

		virtual bool Render(float deltaTime);
		virtual void RenderViewport(std::vector<void*>& texture_handles);

		virtual void OnEvent(Event* p_event);

	private:
	protected:
		bool m_windowOpen = true;
		bool m_openLastFrame = true;
		bool m_isOpen = true;
		std::string m_name;
		ImGuiID dockspace_id;
		bool can_close = true;
		bool first_run = true;
		bool is_focused = true;

	};

}
