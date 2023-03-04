#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "Graphics.h"

// Temp--------------------------------
#include "GBuffer.h"
#include "GDevice.h"
#include "core/events/Events.h"
#include "Camera.h"
#include "resource/ResourceSystem.h"
#include "GFramebuffer.h"
#include "GRenderPass.h"
#include "GSwapchain.h";
#include "Model.h"
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
		void UsePipeline(GPipeline* pipeline);
		void Start();
		void End();
		void ShutDown();
		void OnEvent(Event* p_event);
		
		void Draw(Model* model);

		static Renderer* s_instance;
		static RenderAPI s_api;
		static Renderer* get_instance();
		static RenderAPI get_api();



		// Temp-----------------------------
		eastl::vector<Vertex> m_vertices;
		eastl::vector<uint32_t> m_indices;

		GBuffer* VertexBuffer;
		GBuffer* IndexBuffer;

		GShader* VertShader;
		GShader* FragShader;

		GPipeline* _pipeline;
		Camera* _camera;

		GTexture* _texture;


		GPipeline* _pipeline0;
		GRenderPass* _renderPass;
		GFramebuffer* _framebuffer;
		GSwapchain* _swapChain;

		Viewport _viewPort;
		Rect2D _rect;

		//------------------------------------


	private:
		GContext* m_context;
		GDevice* m_device;

	protected:

	};

}
