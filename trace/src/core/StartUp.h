#pragma once

#include "Core.h"
#include "Coretypes.h"
#include "Enums.h"


namespace trace {

	//TODO: Rename functions so that what each of them would be clear ------------------------------------

	int initialize(int argc, char** argv);

	bool TRACE_API INIT();
	void TRACE_API init(trc_app_data app_data);
	bool TRACE_API _INIT(trc_app_data app_data);


	void TRACE_API _SHUTDOWN(trc_app_data app_data);
	void TRACE_API SHUTDOWN();

	// --------------------------------------------------------------------------------------------------------

}
