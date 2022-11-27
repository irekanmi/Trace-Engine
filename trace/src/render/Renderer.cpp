#include <pch.h>

#include "Renderer.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/platform/OpenGL/OpenGLContext.h"
#include "core/platform/OpenGL/OpenGLDevice.h"
#include "core/platform/Vulkan/VKContext.h"


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
			m_device->Init();
			break;

		}

		case RenderAPI::Vulkan:
		{

			m_context = new VKContext();
			m_context->Init();
			// TODO: replace with vulkan device
			m_device = new OpenGLDevice();

			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return false;
		}

		return true;
	}

	void Renderer::Update(float deltaTime)
	{
		m_context->Update(deltaTime);
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
			m_device->DrawElements(buffer);
		}
		else if (usage & BufferUsage::INDEX_BUFFER)
		{
			m_device->DrawIndexed(buffer);
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