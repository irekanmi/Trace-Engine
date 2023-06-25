#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "Graphics.h"
#include "Commands.h"

// Temp--------------------------------
#include "GBuffer.h"
#include "GDevice.h"
#include "core/events/Events.h"
#include "Camera.h"
#include "resource/ResourceSystem.h"
#include "GFramebuffer.h"
#include "GRenderPass.h"
#include "GSwapchain.h"
#include "SkyBox.h"
#include "GContext.h"
#include "render_graph/RenderGraph.h"
//----------------------------------------


namespace trace {

	class RenderGraph;

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
		void Start();
		void End();
		void ShutDown();
		void OnEvent(Event* p_event);
		GRenderPass* GetRenderPass(RENDERPASS render_pass);
		void Render(float deltaTime);
		


		void DrawMesh(CommandList& cmd_list, Ref<Mesh> mesh, glm::mat4 model);
		void DrawSky(CommandList& cmd_list, SkyBox* sky);

		// Command List
		CommandList BeginCommandList();
		void SubmitCommandList(CommandList& list);
	

		static Renderer* s_instance;
		static Renderer* get_instance();



		// Temp-----------------------------
		Ref<GPipeline> sky_pipeline;
		GRenderPass _renderPass[RENDERPASS::RENDER_PASS_COUNT];
		GFramebuffer _framebuffer;
		GSwapchain _swapChain;
		Viewport _viewPort;
		Rect2D _rect;
		Camera* _camera;
		glm::ivec4 render_mode;
		RenderGraph test_graph;
		//------------------------------------
	private:
		void draw_mesh(CommandParams params);
		void draw_skybox(CommandParams params);

	private:
		GContext g_context;
		GContext* m_context;
		GDevice* m_device;
		GDevice g_device;
		std::vector<CommandList> m_cmdList;
		uint32_t m_listCount;

		friend RenderGraph;

	protected:

	};

}
