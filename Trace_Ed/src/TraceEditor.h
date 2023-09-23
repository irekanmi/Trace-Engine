#pragma once
#include "EditorRenderComposer.h"

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
		Camera editor_cam;
		Light point_light;
		glm::vec2 m_viewportSize;

		static TraceEditor* s_instance;
	protected:
		friend EditorRenderComposer;
	};

}
