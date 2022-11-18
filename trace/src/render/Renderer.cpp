#include <pch.h>

#include "Renderer.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/platform/OpenGL/OpenGLContext.h"

namespace trace {

	Renderer* Renderer::s_instance = nullptr;

	Renderer::Renderer()
		: Object(_STR(Renderer))
	{
	}

	Renderer::~Renderer()
	{
	}

	bool Renderer::Init(RenderAPI api)
	{
	
		GContext::s_API = api;
		switch (api)
		{
		case RenderAPI::OpenGL:
		{
			m_context = new OpenGLContext();
			if (m_context == nullptr)
			{
				TRC_ERROR(" Failed to create a graphics context ");
				return false;
			}
			m_context->Init();
			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return false;
		}

		return true;
	}

	void Renderer::BeginScene()
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Draw(GBuffer* buffer, BufferUsage usage)
	{
		if (usage & BufferUsage::VERTEX_BUFFER)
		{
			m_context->DrawElements(buffer);
		}
		else if (usage & BufferUsage::INDEX_BUFFER)
		{
			m_context->DrawIndexed(buffer);
		}
	}

	Renderer* Renderer::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new Renderer();
		}
		return s_instance;
	}

	RenderAPI Renderer::get_api()
	{
		return GContext::get_render_api();
	}

}