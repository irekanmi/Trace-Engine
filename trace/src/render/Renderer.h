#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "GPUtypes.h"
#include "GBuffer.h"
#include "GDevice.h"


namespace trace {

	class GContext;

	class TRACE_API Renderer : public Object
	{

	public:
		Renderer();
		~Renderer();

		bool Init(RenderAPI api);
		void Update(float deltaTime);
		void BeginFrame();
		void BeginScene();
		void EndScene();
		void EndFrame();
		void Draw(GBuffer* buffer, BufferUsage usage);
		void Draw(GBuffer* buffer);

		void ShutDown();

		static Renderer* s_instance;
		static RenderAPI s_api;
		static Renderer* get_instance();
		static RenderAPI get_api();
	private:
		GContext* m_context;
		GDevice* m_device;

	protected:

	};

}
