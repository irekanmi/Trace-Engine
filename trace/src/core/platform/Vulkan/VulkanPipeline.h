#pragma once

#include "render/GPipeline.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanPipeline : public GPipeline
	{

	public:
		VulkanPipeline(PipelineStateDesc desc);
		~VulkanPipeline();

		virtual bool Initialize() override;

		virtual void SetData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0) override;
		virtual void SetTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t slot = 1, uint32_t index = 0) override;

		virtual void SetData(const eastl::string resource_name, ShaderResourceStage resource_scope, void* data, uint32_t size) override;
		virtual void SetTextureData(const eastl::string resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index = 0) override;

		virtual void SetMultipleData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count, uint32_t slot = 0, uint32_t index = 0) override;
		virtual void SetMultipleTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t count, uint32_t slot = 1, uint32_t index = 0) override;

		virtual void Bind() override;

		virtual void Shutdown() override;

		VKPipeline m_handle;
		char* cache_data;
		char* _mapped_data;
	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;
		GTexture* last_tex_update[3] = {};

	protected:

	};

}
