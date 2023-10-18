#include <pch.h>

#include "GContext.h"
#include "backends/Renderutils.h"

namespace trace {

	GContext::GContext()
	{

	}

	GContext::~GContext()
	{

	}
	void GContext::Update(float deltaTime)
	{
	}
	void GContext::Init()
	{
		RenderFunc::CreateContext(this);
	}
	void GContext::ShutDown()
	{
		RenderFunc::DestroyContext(this);
	}
}