#pragma once

#include "core/Enums.h"
#include "render/Graphics.h"
#include "render/GDevice.h"
#include "VKtypes.h"
#include "core/events/Events.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"



namespace trace {


	class VKDevice : public GDevice
	{

	public:
		VKDevice();
		~VKDevice();

		bool Init();
		void DrawElements(GBuffer* vertex_buffer);
		void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances);
		void DrawIndexed(GBuffer* index_buffer);
		void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances);
		void ShutDown();

		void BindViewport(Viewport view_port);
		void BindRect(Rect2D rect);
		void BindPipeline(GPipeline* pipeline);
		void BindVertexBuffer(GBuffer* buffer);
		void BindIndexBuffer(GBuffer* buffer);
		void Draw(uint32_t start_vertex, uint32_t count);
		void DrawIndexed(uint32_t first_index, uint32_t count);
		void BeginRenderPass(GRenderPass* render_pass, GFramebuffer* frame_buffer);
		void NextSubpass(GRenderPass* render_pass);
		void EndRenderPass(GRenderPass* render_pass);

		bool BeginFrame(GSwapchain* swapchain);
		void EndFrame();

		void OnEvent(Event* p_Event);

	private:


	public:
		VKDeviceHandle* m_handle;
		VKRenderPass* m_activeRanderPass = nullptr;


	private:
		VKHandle* m_instance;





	protected:

	};

}

namespace vk {

	bool __CreateDevice(trace::GDevice* device);
	bool __DestroyDevice(trace::GDevice* device);

}
