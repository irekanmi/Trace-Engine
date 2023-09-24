#pragma once
#include "EditorRenderComposer.h"
#include "panels/HierachyPanel.h"

#include "scene/Scene.h"

namespace trace {

	class TraceEditor
	{

	public:

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float delaTime);
		void RenderViewport(void* texture);

		RenderComposer* GetRenderComposer();

		void OnEvent(Event* p_event);

		static TraceEditor* get_instance();

	private:

	private:
		EditorRenderComposer* m_renderComposer = nullptr;
		HierachyPanel m_hierachyPanel;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera editor_cam;
		Light point_light;
		Ref<Scene> m_currentScene;

		static TraceEditor* s_instance;
	protected:
		friend EditorRenderComposer;
		friend HierachyPanel;
	};

}
