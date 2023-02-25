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

		virtual bool Init() override;
		virtual void DrawElements(GBuffer* vertex_buffer) override;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) override;
		virtual void DrawIndexed(GBuffer* index_buffer) override;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) override;
		virtual void ShutDown() override;

		virtual void UpdateSceneGlobalData(void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0) override;
		virtual void UpdateSceneGlobalData(SceneGlobals data, uint32_t slot = 0, uint32_t index = 0) override;
		virtual void UpdateSceneGlobalTexture(GTexture* texture, uint32_t slot = 1, uint32_t index = 0) override;
		virtual void UpdateSceneGlobalTextures(GTexture* texture, uint32_t count, uint32_t slot = 1, uint32_t index = 0) override;

		virtual void BindViewport(Viewport view_port) override;
		virtual void BindRect(Rect2D rect) override;
		virtual void BindPipeline(GPipeline* pipeline) override;
		virtual void BindVertexBuffer(GBuffer* buffer) override;
		virtual void BindIndexBuffer(GBuffer* buffer) override;
		virtual void Draw(uint32_t start_vertex, uint32_t count) override;
		virtual void DrawIndexed(uint32_t first_index, uint32_t count) override;
		virtual void BeginRenderPass(GRenderPass* render_pass, GFramebuffer* frame_buffer) override;
		virtual void NextSubpass(GRenderPass* render_pass) override;
		virtual void EndRenderPass(GRenderPass* render_pass) override;

		virtual bool BeginFrame(GSwapchain* swapchain) override;
		virtual void EndFrame() override;

		void OnEvent(Event* p_Event);

	private:
		bool recreateSwapchain();


	public:
		VKDeviceHandle* m_handle;
		VKRenderPass* m_activeRanderPass = nullptr;


	private:
		VKHandle* m_instance;





	protected:

	};

}
