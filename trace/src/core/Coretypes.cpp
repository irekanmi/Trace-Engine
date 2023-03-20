#include "pch.h"

#include "core/Core.h"
#include "core/Enums.h"
#include "Coretypes.h"

namespace trace {

	WindowDecl   AppSettings::winprop = WindowDecl();
	WindowType   AppSettings::wintype = WindowType::DEFAULT;
	RenderAPI    AppSettings::graphics_api = RenderAPI::None;
	PlatformAPI  AppSettings::platform_api = PlatformAPI::NO;
	bool         AppSettings::enable_vsync = false;
	bool         AppSettings::windowed = false;

}