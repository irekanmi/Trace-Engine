#include <pch.h>

#include "Renderer.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/platform/OpenGL/OpenGLContext.h"
#include "core/platform/OpenGL/OpenGLDevice.h"
#include "core/platform/Vulkan/VKContext.h"
#include "core/platform/Vulkan/VKDevice.h"


namespace trace {

	Renderer* Renderer::s_instance = nullptr;
	RenderAPI Renderer::s_api = RenderAPI::None;

	Renderer::Renderer()
		: Object(_STR(Renderer))
	{
	}

	Renderer::~Renderer()
	{
	}

	bool Renderer::Init(RenderAPI api)
	{
		bool result = false;
	
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
			m_device = new OpenGLDevice();
			result = m_device->Init();
			break;

		}

		case RenderAPI::Vulkan:
		{

			m_context = new VKContext();
			m_context->Init();

			m_device = new VKDevice();
			result = m_device->Init();

			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return result;
		}

		return result;
	}

	void Renderer::Update(float deltaTime)
	{
		m_context->Update(deltaTime);
	}

	bool Renderer::BeginFrame()
	{
		 return m_device->BeginFrame();
	}

	void Renderer::BeginScene()
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::EndFrame()
	{
		m_device->EndFrame();
	}

	void Renderer::Draw(GBuffer* buffer, BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::VERTEX_BUFFER:
		{
			m_device->DrawElements(buffer);
			break;
		}
		case BufferUsage::INDEX_BUFFER:
		{
			m_device->DrawIndexed(buffer);
			break;
		}
		}
	}

	void Renderer::Draw(GBuffer* buffer)
	{
		BufferUsage buf_use = buffer->GetUsage();
		if (buf_use & BufferUsage::VERTEX_BUFFER)
		{
			m_device->DrawElements(buffer);
		}
		else if (buf_use & BufferUsage::INDEX_BUFFER)
		{
			m_device->DrawIndexed(buffer);
		}
	}

	void Renderer::ShutDown()
	{
		m_device->ShutDown();
		m_context->ShutDown();

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