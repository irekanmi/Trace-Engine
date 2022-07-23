#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef TRC_WINDOWS
	#ifdef TRC_CORE
	#define TRACE_API _declspec(dllexport)
	#else
	#define TRACE_API _declspec(dllimport)
	#endif

#endif
