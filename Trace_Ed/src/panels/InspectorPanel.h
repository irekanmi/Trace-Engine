#pragma once

#include "scene/Entity.h"

namespace trace {

	class TraceEditor;

	class InspectorPanel
	{

	public:
		InspectorPanel();
		~InspectorPanel() {}


		void DrawEntityComponent(Entity entity);
		void SetDrawCallbackFn(std::function<void()> cb, std::function<void()> on_enter, std::function<void()> on_exit);

		TraceEditor* m_editor;
	private:
		std::function<void()> m_drawCallback;
		std::function<void()> m_onExit;
		Entity m_selectedEntity;

	protected:
		friend class TraceEditor;
	};

}
