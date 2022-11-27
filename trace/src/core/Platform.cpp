#include "pch.h"
#include "Platform.h"
#ifdef TRC_WINDOWS
#include <Windows.h>
#endif
#include "core/io/Logging.h"
#include "EASTL/vector.h"

namespace trace {
	PlatformAPI Platform::s_api = PlatformAPI::NO;

	void Platform::OpenDir()
	{

	}

	void* Platform::GetAppHandle()
	{
		switch (s_api)
		{
		case PlatformAPI::WINDOWS:
		{
			HINSTANCE handle = ::GetModuleHandleA(nullptr);
			return handle;
			break;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");
		return nullptr;

	}

	void Platform::ZeroMem(void* dst, uint32_t lenght)
	{
		switch (s_api)
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
		switch (s_api)
		{
		case PlatformAPI::WINDOWS:
		{
			Sleep(milleseconds);
			return;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");
	}

	const char** Platform::GetExtensions(uint32_t& count)
	{

		switch (Platform::s_api)
		{
		case PlatformAPI::WINDOWS:
		{
			const char* ret[] = { "VK_KHR_win32_surface" };
			count = 1;

			return ret;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");

	}

}
