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
#include "render/render_graph/RenderGraph.h"
#include "core/defines.h"


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
	class SkinnedModel;
	class MaterialInstance;
	class Event;
	class Font;
	class ParticleEffectInstance;

	struct RenderObjectData
	{
		glm::mat4 transform = glm::mat4(1.0f);
		Model* object = nullptr;
		MaterialInstance* material = nullptr;
	};
	struct RenderSkinnedObjectData
	{
		glm::mat4 transform = glm::mat4(1.0f);
		SkinnedModel* object = nullptr;
		MaterialInstance* material = nullptr;
		glm::mat4* bone_transforms;
		uint32_t bone_count = 0;
	};

	struct QuadInstanceData
	{
		// IMPORTANT: there is a limit to number of instances in the shader
		std::vector<glm::mat4> transforms;
		std::vector<uint32_t> colors;
	};
	
	struct TextInstanceData
	{
		// IMPORTANT: there is a limit to number of instances in the shader
		std::vector<glm::vec4> positions;
		std::vector<glm::vec4> tex_coords;
	};

	struct RenderGraphFrameData
	{
		Viewport _viewPort;
		Rect2D _rect;
		Camera* _camera = nullptr;

		std::vector<RenderObjectData> m_opaqueObjects;
		std::vector<RenderSkinnedObjectData> m_opaqueSkinnedObjects;

		std::vector<RenderObjectData> m_transparentUnLitObjects;
		std::vector<RenderObjectData> m_partilcleBillBoard;
		uint32_t m_opaqueObjectsSize = 0;

		// Instanced Quad ............................
		std::unordered_map<GTexture*, QuadInstanceData> quad_instances;
		// ............................
		
		// Instanced Text ............................
		std::unordered_map<GTexture*, TextInstanceData> text_instances;
		// ............................


		//SunLights 
		// NOTE: SunLight should not be more than one per scene for optimization reasons
		// NOTE: The light vector stores lights from those that has shadows to does that not
		uint32_t num_shadowed_sun_lights;
		uint32_t num_non_shadowed_sun_lights;
		std::vector<Light> sun_lights;

		uint32_t num_shadowed_point_lights;
		uint32_t num_non_shadowed_point_lights;
		std::vector<Light> point_lights;

		uint32_t num_shadowed_spot_lights;
		uint32_t num_non_shadowed_spot_lights;
		std::vector<Light> spot_lights;

		//Shadow Casters
		std::vector<RenderObjectData> shadow_casters;
		std::vector<RenderSkinnedObjectData> skinned_shadow_casters;

		//Particle Effects
		std::vector<ParticleEffectInstance*> particle_effects;
	};

	class Renderer : public Object
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
		CommandList BeginCommandList(int32_t render_graph_index = 0);
		void SubmitCommandList(CommandList& list, int32_t render_graph_index = 0);
		void BeginScene(CommandList& cmd_list, Camera* _camera, int32_t render_graph_index = 0);
		void EndScene(CommandList& cmd_list, int32_t render_graph_index = 0);
		void DrawMesh(CommandList& cmd_list, Ref<Mesh> _mesh, glm::mat4 model, int32_t render_graph_index = 0);
		void DrawModel(CommandList& cmd_list, Ref<Model> _model, glm::mat4 transform, int32_t render_graph_index = 0);
		void DrawModel(CommandList& cmd_list, Ref<Model> _model, Ref<MaterialInstance> material, glm::mat4 transform, bool cast_shadow, int32_t render_graph_index = 0);
		void DrawSkinnedModel(CommandList& cmd_list, Ref<SkinnedModel> _model, Ref<MaterialInstance> material, glm::mat4 transform, glm::mat4* bone_transforms, uint32_t bone_count, bool cast_shadow, int32_t render_graph_index = 0);
		void DrawSky(CommandList& cmd_list, SkyBox* sky, int32_t render_graph_index = 0);
		void DrawLight(CommandList& cmd_list, Ref<Mesh> _mesh, Light& light_data, LightType light_type, int32_t render_graph_index = 0);
		void AddLight(CommandList& cmd_list, Light& light_data, LightType light_type, int32_t render_graph_index = 0);
		void DrawDebugLine(CommandList& cmd_list, glm::vec3 from, glm::vec3 to, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugLine(CommandList& cmd_list, glm::vec3 p0, glm::vec3 p1, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugCircle(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugSphere(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawString(CommandList& cmd_list, Ref<Font> font, const std::string& text, glm::vec3 color, glm::mat4 _transform, int32_t render_graph_index = 0);
		void DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform, int32_t render_graph_index = 0);
		void DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform, uint32_t color, int32_t render_graph_index = 0);
		void DrawParticleEffect(CommandList& cmd_list, ParticleEffectInstance* particle_effect,int32_t render_graph_index = 0);


		// Getters
		GRenderPass* GetRenderPass(const std::string& pass_name) { return (GRenderPass*)m_avaliablePasses[pass_name]; }
		RenderGraphFrameData* GetRenderGraphData(int32_t render_graph_index) { return &m_renderGraphsData[render_graph_index]; }
		std::string GetRenderPassName(GRenderPass* pass);
		GDevice* GetDevice() { return &g_device; }
		GContext* GetContext() { return &g_context; }
		GSwapchain* GetSwapchain() { return &m_swapChain; }
		uint32_t GetFrameWidth() { return m_frameWidth; }
		uint32_t GetFrameHeight() { return m_frameHeight; }
		std::vector<RenderGraphFrameData>& GetRenderGraphFrameData() { return m_renderGraphsData; }
		RenderComposer* GetRenderComposer() { return m_composer; }
		Model& GetQuadModel();

		std::unordered_map<std::string, void*>& GetAvaliableRenderPasses() { return m_avaliablePasses; }


		void Render(float deltaTime);
		void DrawQuad(int32_t render_graph_index = 0);
		void DrawQuad(glm::mat4 _transform, Ref<GTexture> texture, int32_t render_graph_index = 0);
		void DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture, int32_t render_graph_index = 0);
		void DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture, uint32_t color, int32_t render_graph_index = 0);
		void DrawString_(Font* font, const std::string& text, glm::vec3 color, glm::mat4 _transform, int32_t render_graph_index = 0);
		


		void RenderOpaqueObjects(int32_t render_graph_index = 0);
		void RenderTransparentUnLitObjects(int32_t render_graph_index, RenderGraph* render_graph, uint32_t screen_color_index);
		void RenderQuads(int32_t render_graph_index = 0);
		void RenderTextVerts(int32_t render_graph_index = 0);
		void RenderDebugData(int32_t render_graph_index = 0);
		void RenderParticles(int32_t render_graph_index = 0);
		void RenderParticleBillboard(int32_t render_graph_index = 0);

		static Renderer* get_instance();



		// Per CmdList Objects----------------------------- Temp ---
		Viewport _viewPort;
		Rect2D _rect;

		float exposure;
		//------------------------------------
		
		
	private:
		void draw_mesh(CommandParams& params, int32_t render_graph_index = 0);
		void draw_skybox(CommandParams& params);
		

	private:

	private:
		GSwapchain m_swapChain;
		GContext g_context;
		GDevice g_device;
		RenderComposer* m_composer;
		uint32_t m_frameWidth;
		uint32_t m_frameHeight;
		ClientRenderCallback m_client_render;
		
		uint32_t num_render_graphs = 1;// NOTE: Should be modified at render composer initialiazation
		std::vector<RenderGraphFrameData> m_renderGraphsData;

		std::vector<CommandList> m_cmdList;
		uint32_t m_listCount;
		std::unordered_map<std::string, void*> m_avaliablePasses;

		friend class RenderGraph;

	protected:

	};

}
