#include "pch.h"

#include "StartUp.h"
#include "io/Logging.h"
#include "events/EventsSystem.h"
#include "Enums.h"
#include "input\Input.h"


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

	void SHUTDOWN()
	{
		EventsSystem::shutdown();
		SAFE_DELETE(EventsSystem::get_instance(), EventsSystem);

		Logger::shutdown();
		SAFE_DELETE(Logger::get_instance(), Logger);

	}
}