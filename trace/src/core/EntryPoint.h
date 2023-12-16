#pragma once

#ifndef TRC_ENTRY
#define TRC_ENTRY

#include "Core.h"
#include "Application.h"
#include "StartUp.h"

#ifdef TRC_WINDOWS
extern trace::trc_app_data trace::CreateApp();

int main(int argc, char** argv)
{
	return trace::initialize(argc, argv);
}

#else

#error Trace currently supports only Windows

#endif
#endif
