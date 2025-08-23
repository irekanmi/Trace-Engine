#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
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


		virtual std::vector<Object*> GetEngineSystemsID();
		virtual Window* GetWindow() { return m_Window; }
		UpdateID GetUpdateID() { return m_updateID; }
		uint32_t GetCurrentTick() { return m_currentTick; }

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
		bool m_isRunning = true;
		bool m_isMinimized = false;
		bool m_vsync = false;
		UpdateID m_updateID = 30;//NOTE: The value 30 is given just to allow the animation graph to have a head start
		uint32_t m_currentTick;

	};

	trc_app_data CreateApp();

}

