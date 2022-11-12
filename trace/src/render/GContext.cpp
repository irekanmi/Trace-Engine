#include <pch.h>

#include "GContext.h"

namespace trace {
	GContext* GContext::s_instance = nullptr;
	RenderAPI GContext::s_API = RenderAPI::None;

	GContext::GContext()
	{

	}

	GContext::~GContext()
	{

	}
}