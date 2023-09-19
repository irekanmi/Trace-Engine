#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/GTexture.h"
#include "render/render_graph/RenderGraph.h"

namespace trace {

	class Application;
	class Renderer;
	class GTexture;
	struct RenderGraphResource;

	// Initialiazation
	typedef bool (*__InitUIRenderBackend)(Application*, Renderer*);
	typedef bool (*__UINewFrame)();
	typedef bool (*__UIEndFrame)();
	typedef bool (*__GetDrawTextureHandle)(GTexture*, void*&);
	typedef bool (*__GetDrawRenderGraphTextureHandle)(RenderGraphResource*, void*&);
	typedef bool (*__UIRenderFrame)(Renderer*);
	typedef bool (*__ShutdownUIRenderBackend)();


	class TRACE_API UIFuncLoader
	{
	public:
		static bool LoadImGuiFunc();
	private:
	protected:
	};

	class TRACE_API UIFunc
	{

	public:
		static bool InitUIRenderBackend(Application* application, Renderer* renderer);
		static bool UINewFrame();
		static bool UIEndFrame();
		static bool UIRenderFrame(Renderer* renderer);
		static bool ShutdownUIRenderBackend();
		static bool GetDrawTextureHandle(GTexture* texture, void*& out_handle);
		static bool GetDrawRenderGraphTextureHandle(RenderGraphResource* texture, void*& out_handle);

	private:
		static __InitUIRenderBackend _initUIRenderBackend;
		static __UINewFrame _uiNewFrame;
		static __UIEndFrame _uiEndFrame;
		static __UIRenderFrame _uiRenderFrame;
		static __ShutdownUIRenderBackend _shutdownUIRenderBackend;
		static __GetDrawTextureHandle _getDrawTextureHandle;
		static __GetDrawRenderGraphTextureHandle _getDrawRenderGraphTextureHandle;

	protected:
		friend UIFuncLoader;

	};

}
