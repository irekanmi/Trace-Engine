#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "Graphics.h"
#include "Commands.h"
#include "resource/Ref.h"
#include "core/Coretypes.h"
#include "GBuffer.h"
#include "GDevice.h"
#include "GSwapchain.h"
#include "GContext.h"


// Temp--------------------------------
#include "Camera.h"
//----------------------------------------


namespace trace {

	class RenderGraph;
	class RenderComposer;
	class SkyBox;
	class GRenderPass;
	class Mesh;
	class Model;
	class MaterialInstance;
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
		GTexture* tex;
	};

	

	class TRACE_API Renderer : public Object
	{
	public:
		Renderer();
		~Renderer();

		bool Init(trc_app_data app_data);
		void Update(float deltaTime);
		bool BeginFrame();
		void EndFrame();
		void Start();
		void End();
		void ShutDown();
		void OnEvent(Event* p_event);


		// Command List
		CommandList BeginCommandList();
		void SubmitCommandList(CommandList& list);
		void BeginScene(CommandList& cmd_list, Camera* _camera);
		void EndScene(CommandList& cmd_list);
		void DrawMesh(CommandList& cmd_list, Ref<Mesh> _mesh, glm::mat4 model);
		void DrawModel(CommandList& cmd_list, Ref<Model> _model, glm::mat4 transform);
		void DrawModel(CommandList& cmd_list, Ref<Model> _model, Ref<MaterialInstance> material, glm::mat4 transform);
		void DrawSky(CommandList& cmd_list, SkyBox* sky);
		void DrawLight(CommandList& cmd_list, Ref<Mesh> _mesh, Light& light_data, LightType light_type);
		void AddLight(CommandList& cmd_list, Light& light_data, LightType light_type);
		void DrawDebugLine(CommandList& cmd_list, glm::vec3 from, glm::vec3 to);
		void DrawDebugLine(CommandList& cmd_list, glm::vec3 p0, glm::vec3 p1, glm::mat4 transform);
		void DrawDebugCircle(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform);
		void DrawDebugSphere(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform);
		void DrawString(CommandList& cmd_list, Ref<Font> font, const std::string& text, glm::vec3 color, glm::mat4 _transform);
		void DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform);


		// Getters
		GRenderPass* GetRenderPass(const std::string& pass_name) { return (GRenderPass*)_avaliable_passes[pass_name]; }
		std::string GetRenderPassName(GRenderPass* pass);
		GDevice* GetDevice() { return &g_device; }
		GContext* GetContext() { return &g_context; }
		GSwapchain* GetSwapchain() { return &m_swapChain; }
		uint32_t GetFrameWidth() { return m_frameWidth; }
		uint32_t GetFrameHeight() { return m_frameHeight; }


		void Render(float deltaTime);
		void DrawQuad();
		void DrawQuad(glm::mat4 _transform, Ref<GTexture> texture);
		void DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture);
		void DrawString(Font* font, const std::string& text, glm::mat4 _transform);
		void DrawString_(Font* font, const std::string& text, glm::vec3 color, glm::mat4 _transform);


		void RenderOpaqueObjects();
		void RenderLights();
		void RenderQuads();
		void RenderTexts();
		void RenderTextVerts();
		void RenderDebugData();

		static Renderer* s_instance;
		static Renderer* get_instance();



		// Per CmdList Objects----------------------------- Temp ---
		Viewport _viewPort;
		Rect2D _rect;
		Camera* _camera = nullptr;

		// light data
		std::vector<glm::vec4> light_positions;
		std::vector<glm::vec4> light_directions;
		std::vector<glm::vec4> light_colors;
		std::vector<glm::vec4> light_params1s; // x: constant, y: linear, z:quadratic, w: innerCutOff
		std::vector<glm::vec4> light_params2s; // x: outerCutOff, y: intensity, z:null, w: null

		std::vector<Light> lights;
		glm::ivec4 light_data;
		float exposure;
		bool text_verts = false;
		//------------------------------------
		std::unordered_map<std::string, void*> _avaliable_passes;
		
	private:
		void draw_mesh(CommandParams& params);
		void draw_skybox(CommandParams& params);
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


		struct RenderObjectData
		{
			glm::mat4 transform = glm::mat4(1.0f);
			Model* object = nullptr;
			MaterialInstance* material = nullptr;
		};

	private:
		// Debug Renderering
		struct DebugData
		{
			DebugData() {};
			~DebugData() {};

			std::vector<glm::vec4> positions;
			uint32_t vert_count = 0;
			Ref<GPipeline> m_linePipeline;
		};
		DebugData* m_debug;		

	private:
		GSwapchain m_swapChain;
		GContext g_context;
		GDevice g_device;
		RenderComposer* m_composer;
		uint32_t m_frameWidth;
		uint32_t m_frameHeight;
		ClientRenderCallback m_client_render;
		std::vector<RenderObjectData> m_opaqueObjects;

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

		//Text Renderering ..............................
		std::vector<std::vector<TextVertex>> text_vertices;
		std::vector<GTexture*> text_atlases;
		Ref<GPipeline> text_pipeline;
		std::unordered_set<uint32_t> bound_text_atlases;
		GBuffer text_buffer;
		// ..............................................

		friend class RenderGraph;

	protected:

	};

}
