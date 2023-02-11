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

			// TODO: Implement OpenGl Device
			//m_device = new OpenGLDevice();
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

	void Renderer::UsePipeline(GPipeline* pipeline)
	{
		m_device->m_pipeline = pipeline;
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

	void Renderer::UpdateSceneGlobalData(void* data, uint32_t size, uint32_t slot, uint32_t index)
	{
		m_device->UpdateSceneGlobalData(data, size, slot, index);
	}

	void Renderer::UpdateSceneGlobalData(SceneGlobals data, uint32_t slot, uint32_t index)
	{
		m_device->UpdateSceneGlobalData(data, slot, index);
	}

	void Renderer::UpdateSceneGlobalTexture(GTexture* texture, uint32_t slot, uint32_t index)
	{
		m_device->UpdateSceneGlobalTexture(texture, slot, index);
	}

	void Renderer::BindPipeline(GPipeline* pipeline)
	{
		m_device->BindPipeline(pipeline);
	}

	void Renderer::BindVertexBuffer(GBuffer* buffer)
	{
		if (TRC_HAS_FLAG(buffer->GetUsage(), BufferUsage::VERTEX_BUFFER))
		{
			m_device->BindVertexBuffer(buffer);
		}
	}

	void Renderer::BindIndexBuffer(GBuffer* buffer)
	{
		if (TRC_HAS_FLAG(buffer->GetUsage(), BufferUsage::INDEX_BUFFER))
		{
			m_device->BindIndexBuffer(buffer);
		}

	}

	void Renderer::Draw(uint32_t start_vertex, uint32_t count)
	{
		m_device->Draw(start_vertex, count);
	}

	void Renderer::DrawIndexed(uint32_t first_index, uint32_t count)
	{
		m_device->DrawIndexed(first_index, count);
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