#pragma once



#ifdef TRC_WINDOWS
	#ifdef TRC_CORE
	#define TRACE_API _declspec(dllexport)
	#else
	#define TRACE_API _declspec(dllimport)
	#endif

#endif

#define SAFE_DELETE(a) delete a; a = nullptr;