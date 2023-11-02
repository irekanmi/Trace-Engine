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
		void SetDrawCallbackFn(std::function<void()> cb) { m_drawCallback = cb; }

		TraceEditor* m_editor;
	private:
		std::function<void()> m_drawCallback;
		Entity m_selectedEntity;

	protected:
		friend class TraceEditor;
	};

}
