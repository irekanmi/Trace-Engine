#pragma once

#include "core/Core.h"
#include "core/Enums.h"

namespace trace {

	class Application;
	class Renderer;

	// Initialiazation
	typedef bool (*__InitUIRenderBackend)(Application*, Renderer*);
	typedef bool (*__UINewFrame)();
	typedef bool (*__UIEndFrame)();
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

	private:
		static __InitUIRenderBackend _initUIRenderBackend;
		static __UINewFrame _uiNewFrame;
		static __UIEndFrame _uiEndFrame;
		static __UIRenderFrame _uiRenderFrame;
		static __ShutdownUIRenderBackend _shutdownUIRenderBackend;

	protected:
		friend UIFuncLoader;

	};

}
