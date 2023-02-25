#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
#include "Layer.h"
#include "Coretypes.h"
#include "EASTL/vector.h"

// temp -------------------------

#include "resource/ResourceSystem.h"

#include "render/Graphics.h"
#include "render/GBuffer.h"
#include "render/Graphics.h"
#include "render/GPipeline.h"
#include "render/GShader.h"
#include "render/Camera.h"
#include "render/GTexture.h"

// ------------------------------------


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
		virtual eastl::vector<Object*> GetEngineSystemsID();
		virtual Window* GetWindow() { return m_Window; }

		static Application* get_instance();
		static Application* s_instance;
		static WindowType win_type;

	private:
		ClientEndCallback     m_client_end;
		ClientStartCallback   m_client_start;
		ClientUpdateCallback  m_client_update;


	protected:
		Window* m_Window;
		bool m_isRunning = true;
		LayerStack* m_LayerStack;

	};

	trc_app_data CreateApp();

}

