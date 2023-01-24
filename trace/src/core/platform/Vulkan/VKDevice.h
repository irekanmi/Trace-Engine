#pragma once

#include "core/Enums.h"
#include "render/GPUtypes.h"
#include "render/GDevice.h"
#include "VKtypes.h"
#include "core/events/Events.h"

namespace trace {

	class VKDevice : public GDevice
	{

	public:
		VKDevice();
		~VKDevice();

		virtual void Init() override;
		virtual void DrawElements(GBuffer* vertex_buffer) override;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) override;
		virtual void DrawIndexed(GBuffer* index_buffer) override;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) override;
		virtual void ShutDown() override;

		virtual bool BeginFrame() override;
		virtual void EndFrame() override;

		void OnEvent(Event* p_Event);

	private:
		bool recreateSwapchain();

	private:
		VKDeviceHandle* m_handle;
		VKHandle* m_instance;


	protected:

	};

}
