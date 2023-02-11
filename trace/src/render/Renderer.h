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
		bool BeginFrame();
		void BeginScene();
		void EndScene();
		void EndFrame();
		void UsePipeline(GPipeline* pipeline);

		// TODO
		void Draw(GBuffer* buffer, BufferUsage usage);
		void Draw(GBuffer* buffer);

		// TODO: Renderer API is going to change
		virtual void UpdateSceneGlobalData(void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0);
		virtual void UpdateSceneGlobalData(SceneGlobals data, uint32_t slot = 0, uint32_t index = 0);
		virtual void UpdateSceneGlobalTexture(GTexture* texture, uint32_t slot = 1, uint32_t index = 0);
		virtual void BindPipeline(GPipeline* pipeline);
		virtual void BindVertexBuffer(GBuffer* buffer);
		virtual void BindIndexBuffer(GBuffer* buffer);
		virtual void Draw(uint32_t start_vertex, uint32_t count);
		virtual void DrawIndexed(uint32_t first_index, uint32_t count);

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
