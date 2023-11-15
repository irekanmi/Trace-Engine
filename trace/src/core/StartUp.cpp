#include "pch.h"

#include "StartUp.h"
#include "io/Logging.h"
#include "events/EventsSystem.h"
#include "input/Input.h"
#include "render/GContext.h"
#include "render/Renderer.h"
#include "core/Platform.h"
#include "core/Application.h"
#include "resource/ResourceSystem.h"
#include "core/memory/MemoryManager.h"
#include "backends/Physicsutils.h"


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

		if (MemoryManager::get_instance() == nullptr)
		{
			TRC_ERROR("failed to create Memory Manager");
			return false;
		}
		
		if (!MemoryManager::get_instance()->Init())
		{
			TRC_ERROR("Memory Manager failed to initialize");
			return false;
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

	void TRACE_API init(trc_app_data app_data)
	{

		AppSettings::winprop      = app_data.winprop;
		AppSettings::wintype      = app_data.wintype;
		AppSettings::graphics_api = app_data.graphics_api;
		AppSettings::platform_api = app_data.platform_api;
		AppSettings::enable_vsync = app_data.enable_vsync;
		AppSettings::windowed = app_data.windowed;


	}

	bool TRACE_API _INIT(trc_app_data app_data)
	{
		
		if (!Renderer::get_instance()->Init(app_data))
		{
			TRC_ERROR("Failed to initialize renderer");
			return false;
		}

		if (!ResourceSystem::Init())
		{
			TRC_ERROR("Failed to initialize Resource System");
			return false;
		}

		//Physics Initialization
		//TODO: Allow physics to be dynamic
		{
			if (!PhysicsFuncLoader::LoadPhysxFunctions())
			{
				TRC_ERROR("Failed to load physics functions");
			}
			else
			{
				if (!PhysicsFunc::InitPhysics3D())
				{
					TRC_ERROR("Failed initialize 3D Physics");
				}
			}
		};

		return true;
	}

	void TRACE_API _SHUTDOWN(trc_app_data app_data)
	{
		PhysicsFunc::ShutdownPhysics3D();

		ResourceSystem::ShutDown();
		Renderer::get_instance()->ShutDown();
		SAFE_DELETE(Renderer::get_instance(), Renderer);
		return;
	}

	void SHUTDOWN()
	{
		EventsSystem::shutdown();
		SAFE_DELETE(EventsSystem::get_instance(), EventsSystem);

		Logger::get_instance()->Shutdown();
		SAFE_DELETE(Logger::get_instance(), Logger);

	}
}