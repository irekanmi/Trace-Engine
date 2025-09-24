#include "pch.h"
#include "Platform.h"
#ifdef TRC_WINDOWS
#include <Windows.h>
#endif
#include "core/io/Logging.h"
#include "EASTL/vector.h"
#include <dos.h>
#include "Coretypes.h"


namespace trace {


	void Platform::OpenDir()
	{

	}

	void* Platform::GetAppHandle()
	{
		switch (AppSettings::platform_api)
		{
		case PlatformAPI::WINDOWS:
		{
			HINSTANCE handle = ::GetModuleHandleA(nullptr);
			return handle;
			break;
		}
		}

		TRC_ASSERT(false, "A vaild platform type has to be specified ");
		return nullptr;

	}

	void Platform::ZeroMem(void* dst, uint32_t lenght)
	{
		switch (AppSettings::platform_api)
		{
		case PlatformAPI::WINDOWS:
		{
			::ZeroMemory(dst, lenght);
			return;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");
	}

	void Platform::Sleep(float milleseconds)
	{
		switch (AppSettings::platform_api)
		{
		case PlatformAPI::WINDOWS:
		{
			::DWORD value = (::DWORD)milleseconds;
			::Sleep(value);
			return;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");
	}

	void Platform::GetExtensions(uint32_t& count, eastl::vector<const char*>& extensions)
	{

		switch (AppSettings::platform_api)
		{
		case PlatformAPI::WINDOWS:
		{
			extensions.push_back("VK_KHR_win32_surface");
			count = 1;

			return;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");

	}

}
