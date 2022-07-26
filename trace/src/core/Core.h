#pragma once



#ifdef TRC_WINDOWS

//#ifndef TRC_DLL_BUILD || TRC_APP
//	#ifdef TRC_CORE
//		#define TRACE_API _declspec(dllexport)
//		#else
//		#define TRACE_API _declspec(dllimport)
//	#endif
//#else
//#define TRACE_API
//#endif

#define TRACE_API 



#endif

#define SAFE_DELETE(a) delete a; a = nullptr;