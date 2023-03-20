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
//----------------------------------------


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
		void Start();
		void End();
		void ShutDown();
		void OnEvent(Event* p_event);
		GRenderPass* GetRenderPass(RENDERPASS render_pass);
		void Render(float deltaTime);
		


		void DrawMesh(CommandList& cmd_list, Ref<Mesh> mesh);
		void DrawSky(CommandList& cmd_list, SkyBox* sky);

		// Command List
		CommandList BeginCommandList();
		void SubmitCommandList(CommandList& list);
	

		static Renderer* s_instance;
		static Renderer* get_instance();



		// Temp-----------------------------
		eastl::vector<Vertex> m_vertices;
		eastl::vector<uint32_t> m_indices;

		GBuffer* VertexBuffer;
		GBuffer* IndexBuffer;

		GShader* VertShader;
		GShader* FragShader;

		Ref<GPipeline> skybox_pipeline;
		GPipeline* reflect_pipeline;
		Camera* _camera;

		GTexture* _texture;


		Ref<GPipeline> _pipeline;
		GRenderPass* _renderPass[RENDERPASS::RENDER_PASS_COUNT];
		GFramebuffer* _framebuffer;
		GSwapchain* _swapChain;

		Viewport _viewPort;
		Rect2D _rect;
		SkyBox _sky;
	

		//------------------------------------
	private:
		void Draw_Mesh(CommandParams params);
		void DrawSkyBox(CommandParams params);

	private:
		GContext* m_context;
		GDevice* m_device;
		std::vector<CommandList> m_cmdList;
		uint32_t m_listCount;

	protected:

	};

}
