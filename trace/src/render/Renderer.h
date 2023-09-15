#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "Graphics.h"
#include "Commands.h"
#include "resource/Ref.h"


// Temp--------------------------------
#include "GBuffer.h"
#include "GDevice.h"
#include "Camera.h"
#include "GSwapchain.h"
#include "GContext.h"
#include "render_graph/RenderGraph.h"
#include "RenderComposer.h"
//----------------------------------------


namespace trace {

	class RenderGraph;
	class SkyBox;
	class GRenderPass;
	class Mesh;
	class Model;
	class Event;
	class Font;

	struct BatchInfo
	{
		uint32_t max_units;
		uint32_t max_texture_units;
		uint32_t current_unit;
		uint32_t current_texture_unit;
		uint32_t current_index;
		std::vector<GTexture*> textures;
		std::vector<glm::vec4> positions;
		std::vector<glm::vec4> tex_coords;
	};

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
		GRenderPass* GetRenderPass(const std::string& pass_name) { return (GRenderPass*)_avaliable_passes[pass_name]; }
		GDevice* GetDevice() { return &g_device; }
		GContext* GetContext() { return &g_context; }
		void Render(float deltaTime);
		void DrawQuad();
		void DrawQuad(glm::mat4 transform, Ref<GTexture> texture);
		void DrawString(Ref<Font> font, const std::string& text, glm::mat4 transform);


		void RenderOpaqueObjects();
		void RenderLights();
		void RenderQuads();
		void RenderTexts();
		


		void DrawMesh(CommandList& cmd_list, Ref<Mesh> mesh, glm::mat4 model);
		void DrawSky(CommandList& cmd_list, SkyBox* sky);

		// Command List
		CommandList BeginCommandList();
		void SubmitCommandList(CommandList& list);
	

		static Renderer* s_instance;
		static Renderer* get_instance();



		// Temp-----------------------------
		Viewport _viewPort;
		Rect2D _rect;
		Camera* _camera;
		glm::ivec4 render_mode;
		std::unordered_map<std::string, void*> _avaliable_passes;
		Light lights[MAX_LIGHT_COUNT];
		glm::ivec4 light_data;
		GBuffer quadBuffer;
		float exposure;
		//------------------------------------
		
	private:
		void draw_mesh(CommandParams params);
		void draw_skybox(CommandParams params);
		// Batch rendering quads ..............................
		void create_quad_batch();
		void flush_current_quad_batch();
		void destroy_quad_batchs();
		// ....................................................

		// Batch rendering text ..............................
		void create_text_batch();
		void flush_current_text_batch();
		void destroy_text_batchs();
		// ....................................................

	private:
		GSwapchain m_swapChain;
		GContext g_context;
		GDevice g_device;
		RenderComposer* m_composer;
		uint32_t m_frameWidth;
		uint32_t m_frameHeight;
		std::vector<std::pair<glm::mat4, Model*>> m_opaqueObjects;
		uint32_t m_opaqueObjectsSize;
		SkyBox* current_sky_box;
		std::vector<CommandList> m_cmdList;
		uint32_t m_listCount;

		// Batch rendering quads ..............................
		std::vector<BatchInfo> quadBatches;
		Ref<GPipeline> quadBatchPipeline;
		std::unordered_set<uint32_t> boundQuadTextures;
		uint32_t current_quad_batch = 0;
		uint32_t num_avalible_quad_batch = 0;
		// ....................................................

		// Batch rendering text ..............................
		std::vector<BatchInfo> textBatches;
		Ref<GPipeline> textBatchPipeline;
		std::unordered_set<uint32_t> boundTextTextures;
		uint32_t current_text_batch = 0;
		uint32_t num_avalible_text_batch = 0;
		// ....................................................


		friend RenderGraph;
		friend RenderComposer;

	protected:

	};

}
