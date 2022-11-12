#include "pch.h"

#include "StartUp.h"
#include "io/Logging.h"
#include "events/EventsSystem.h"
#include "Enums.h"
#include "input\Input.h"
#include "render/GContext.h"
#include "platform/OpenGL/OpenGLContext.h"


namespace trace {

	bool INIT()
	{
		if (Logger::get_instance() == nullptr)
		{
			printf("{ERROR} -> failed to initialize Logger");
			return false;
		}
		else
		{
			Logger::get_instance()->EnableFileLogging();
#ifdef TRC_DEBUG_BUILD
			Logger::get_instance()->set_log_level(LogLevel::trace);
#else
			Logger::get_instance()->set_log_level(LogLevel::warn);
#endif
		}

		if (EventsSystem::get_instance() == nullptr)
		{
			TRC_ERROR("Events System failed to initialize");
			return false;
		}

		if (InputSystem::get_instance() == nullptr)
		{
			TRC_ERROR("Input System failed to initialize");
			return false;
		}

		return true;
	}

	bool TRACE_API _INIT(trc_app_data app_data)
	{
		RenderAPI api = app_data.graphics_api;
		GContext::s_API = api;

		switch (GContext::get_render_api())
		{
		case RenderAPI::OpenGL:
		{
			GContext::s_instance = new OpenGLContext();
			if (GContext::s_instance == nullptr)
			{
				TRC_ERROR(" Failed to create a graphics context ");
				return false;
			}
			GContext::s_instance->Init();
			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return false;
		}
		

		return true;
	}

	void TRACE_API _SHUTDOWN(trc_app_data app_data)
	{

		SAFE_DELETE(GContext::get_instance(), GContext);
		return;
	}

	void SHUTDOWN()
	{
		EventsSystem::shutdown();
		SAFE_DELETE(EventsSystem::get_instance(), EventsSystem);

		Logger::shutdown();
		SAFE_DELETE(Logger::get_instance(), Logger);

	}
}