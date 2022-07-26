#pragma once



#ifdef TRC_WINDOWS

#ifdef TRC_DLL_BUILD

	#ifdef TRC_CORE
	#define TRACE_API _declspec(dllexport)
	#else
	#define TRACE_API _declspec(dllimport)
	#endif

#else
	#define TRACE_API
#endif


#endif

