#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
#include "Layer.h"
#include "Coretypes.h"
#include "Clock.h"




namespace trace
{
	/// It represent an instance of the running application.
	/// 
	/// 
	class TRACE_API Application
	{

	public:
		Application();
		virtual ~Application();

		virtual bool Init(trc_app_data appData);
		virtual void Shutdown();

		/// It gets called after the object has been created
		/// 
		/// 
		virtual void Start();
		virtual void Run();
		virtual void End();

		virtual void OnEvent(Event* p_event);


		virtual void PushLayer(Layer* layer);
		virtual void PushOverLay(Layer* layer);
		virtual void PopLayer(Layer* layer);
		virtual void PopOverLay(Layer* layer);
		virtual std::vector<Object*> GetEngineSystemsID();
		virtual Window* GetWindow() { return m_Window; }

		Clock& GetClock() { return m_clock; }

		static Application* get_instance();

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
		bool m_isMinimized = false;
		bool m_vsync = false;

	};

	trc_app_data CreateApp();

}

