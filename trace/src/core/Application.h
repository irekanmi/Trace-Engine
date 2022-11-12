#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
#include "Layer.h"
#include "Coretypes.h"
#include "render/GPUtypes.h"
#include "EASTL/vector.h"


namespace trace
{
	

	class TRACE_API Application
	{

	public:
		Application(trc_app_data appData);
		virtual ~Application();

		virtual void Start();
		virtual void Run();
		virtual void End();

		virtual void OnEvent(Event* p_event);


		virtual void PushLayer(Layer* layer);
		virtual void PushOverLay(Layer* layer);
		virtual void PopLayer(Layer* layer);
		virtual void PopOverLay(Layer* layer);

		static Application* get_instance();
		static Application* s_instance;
	private:
		ClientEndCallback     m_client_end;
		ClientStartCallback   m_client_start;
		ClientUpdateCallback  m_client_update;

		eastl::vector<Vertex> m_vertices;
		eastl::vector<uint32_t> m_indices;

		// Temp
		uint32_t VertexBuffer;
		uint32_t IndexBuffer;

		uint32_t Shader;

	protected:
		Window* m_Window;
		bool m_isRunning = true;
		LayerStack* m_LayerStack;

	};

	trc_app_data CreateApp();

}

