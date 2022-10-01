#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"
#include "Layer.h"


namespace trace
{
	typedef void(*ClientUpdateCallback)(float deltaTime);
	typedef void(*ClientStartCallback)();
	typedef void(*ClientEndCallback)();

	struct trc_app_data
	{
		trc_app_data(){}
		WindowDecl winprop;
		WindowType wintype;
		bool windowed;
		ClientEndCallback client_end;
		ClientStartCallback client_start;
		ClientUpdateCallback client_update;
	};

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
	protected:
		Window* m_Window;
		bool m_isRunning = true;
		LayerStack* m_LayerStack;

	};

	trc_app_data CreateApp();

}

