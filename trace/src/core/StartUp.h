#pragma once

#include "Core.h"
#include "Coretypes.h"

namespace trace {

	bool TRACE_API INIT();
	bool TRACE_API _INIT(trc_app_data app_data);


	void TRACE_API _SHUTDOWN(trc_app_data app_data);
	void TRACE_API SHUTDOWN();

}
