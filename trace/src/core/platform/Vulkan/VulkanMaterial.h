#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Material.h"
#include "VKtypes.h"

namespace trace {


	class TRACE_API VulkanMaterial : public MaterialInstance
	{

	public:
		VulkanMaterial();
		~VulkanMaterial();

		virtual bool Init(Ref<GPipeline> pipeline, Material material) override;
		virtual void Apply() override;

	private:
		VkDescriptorSet m_sets[3];
		VKHandle* m_instance;
		VKDeviceHandle* m_device;
	protected:

	};

}
