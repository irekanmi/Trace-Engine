#pragma once

#include "core/Core.h"
#include "GBuffer.h"
#include "GPipeline.h"
#include "Graphics.h"
#include "GTexture.h"


namespace trace {


	class TRACE_API GDevice
	{

	public:
		GDevice();
		virtual ~GDevice();

		virtual bool Init() = 0;
		virtual void DrawElements(GBuffer* vertex_buffer) = 0;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) = 0;
		virtual void DrawIndexed(GBuffer* index_buffer) = 0;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) = 0;
		virtual void ShutDown() = 0;

		virtual void UpdateSceneGlobalData(void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0) = 0;
		virtual void UpdateSceneGlobalData(SceneGlobals data, uint32_t slot = 0, uint32_t index = 0) = 0;
		virtual void UpdateSceneGlobalTexture(GTexture* texture, uint32_t slot = 1, uint32_t index = 0) = 0;
		virtual void UpdateSceneGlobalTextures(GTexture* texture, uint32_t count, uint32_t slot = 1, uint32_t index = 0) = 0;
		virtual void BindPipeline(GPipeline* pipeline) = 0;
		virtual void BindVertexBuffer(GBuffer* buffer) = 0;
		virtual void BindIndexBuffer(GBuffer* buffer) = 0;
		virtual void Draw(uint32_t start_vertex, uint32_t count) = 0;
		virtual void DrawIndexed(uint32_t first_index, uint32_t count) = 0;


		virtual bool BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void* GetNativePtr() { return nullptr; }

		GPipeline* m_pipeline = nullptr;
	private:
	protected:

	};

}
