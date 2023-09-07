#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
#include "Layer.h"
#include "Coretypes.h"
#include "Clock.h"




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

	private:
		ClientEndCallback     m_client_end;
		ClientStartCallback   m_client_start;
		ClientUpdateCallback  m_client_update;

		

	protected:
		Clock m_clock;
		float m_lastTime = 0.0f;
		Window* m_Window;
		LayerStack* m_LayerStack;
		bool m_isRunning = true;
		bool m_vsync = false;

	};

	trc_app_data CreateApp();

}

