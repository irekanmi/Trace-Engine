#pragma once

#include "core/Core.h"
#include "core/Enums.h"

namespace trace {

	class Application;
	class Renderer;

	// Initialiazation
	typedef bool (*__InitUIRenderBackend)(Application*, Renderer*);
	typedef bool (*__UIBeginFrame)();
	typedef bool (*__UIEndFrame)();
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
		static bool UIBeginFrame();
		static bool UIEndFrame();
		static bool ShutdownUIRenderBackend();

	private:
		static __InitUIRenderBackend _initUIRenderBacked;
		static __UIBeginFrame _uiBeginFrame;
		static __UIEndFrame _uiEndFrame;
		static __ShutdownUIRenderBackend _shutdownUIRenderBackend;

	protected:
		friend UIFuncLoader;

	};

}
