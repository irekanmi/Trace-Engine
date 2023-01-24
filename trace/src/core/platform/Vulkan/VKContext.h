#pragma once

#include "render/GContext.h"
#include "vulkan/vulkan.h"
#include "VKtypes.h"


namespace trace {


	class VKContext : public GContext
	{
	public:
		VKContext();
		~VKContext();

		virtual void Update(float deltaTime) override;

		virtual void Init() override;
		virtual void ShutDown() override;

	private:
		VKHandle* m_handle;

	protected:

	};

}

