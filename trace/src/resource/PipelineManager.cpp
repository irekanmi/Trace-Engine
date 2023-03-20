#include "pch.h"

#include "PipelineManager.h"
#include "core/Coretypes.h"
#include "core/platform/Vulkan/VulkanPipeline.h"
#include "render/Renderer.h"
#include "core/FileSystem.h"
#include "render/GShader.h"

namespace trace {

	PipelineManager* PipelineManager::s_instance = nullptr;

	PipelineManager::PipelineManager()
	{
	}
	PipelineManager::PipelineManager(uint32_t max_entires)
	{
	}
	PipelineManager::~PipelineManager()
	{
	}
	bool PipelineManager::Init(uint32_t max_entries)
	{
		m_numEntries = max_entries;
		m_hashtable.Init(m_numEntries);
		m_hashtable.Fill(INVALID_ID);

		switch (AppSettings::graphics_api)
		{
		case RenderAPI::Vulkan:
		{
			m_pipelineTypeSize = sizeof(VulkanPipeline);

			//TODO: Use a custom allocator for allocation
			m_pipelines = new unsigned char[m_pipelineTypeSize * m_numEntries];

			if (!m_pipelines)
			{
				TRC_ERROR("Failed to allocate memory for Pipeline Manager");
				return false;
			}

			VulkanPipeline* pipelines = (VulkanPipeline*)m_pipelines;

			for (uint32_t i = 0; i < m_numEntries; i++)
			{
				pipelines[i].m_id = INVALID_ID;
			}

			break;
		}
		default:
		{
			TRC_ASSERT(false, "Render API Texture not suppoted");
			return false;
			break;
		}
		}

		return true;
	}
	void PipelineManager::ShutDown()
	{
		if (m_pipelines)
		{
			standard_pipeline.~Ref();
			skybox_pipeline.~Ref();
			delete[] m_pipelines;
			m_pipelines = nullptr;
		}

	}
	bool PipelineManager::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{

		if (auto_fill || desc.render_pass == nullptr)
		{
			desc.render_pass = Renderer::get_instance()->GetRenderPass(desc._renderPass);
		}

		switch (AppSettings::graphics_api)
		{
		case RenderAPI::Vulkan:
		{
			VulkanPipeline* pipelines = (VulkanPipeline*)m_pipelines;
			bool found = false;
			for (uint32_t i = 0; i < m_numEntries; i++)
			{
				if (pipelines[i].m_id == INVALID_ID)
				{
					VulkanPipeline* pipeline = &pipelines[i];
					new(pipeline) VulkanPipeline(desc);
					pipeline->m_id = i;
					m_hashtable.Set(name, i);
					found = true;
					break;
				}
			}

			if (!found)
			{
				TRC_ERROR("Failed to create pipeline %s", name.c_str());
				return false;
			}

			break;
		}
		default:
		{
			TRC_ASSERT(false, "Render API Texture not suppoted");
			return false;
			break;
		}
		}

		return true;
	}
	GPipeline* PipelineManager::GetPipeline(const std::string& name)
	{
		uint32_t hash = m_hashtable.Get(name);

		if (hash != INVALID_ID)
		{
			switch (AppSettings::graphics_api)
			{
			case RenderAPI::Vulkan:
			{
				VulkanPipeline* pipelines = (VulkanPipeline*)m_pipelines;
				return &pipelines[hash];
				break;
			}
			default:
			{
				TRC_ASSERT(false, "Render API Texture not suppoted");
				return nullptr;
				break;
			}
			}
		}

		return nullptr;
	}
	Ref<GPipeline> PipelineManager::GetDefault(const std::string& name)
	{
		if (name == "standard")
		{
			return standard_pipeline;
		}
		if (name == "skybox")
		{
			return skybox_pipeline;
		}

		return Ref<GPipeline>();
	}
	void PipelineManager::Unload(GPipeline* pipeline)
	{
		if (pipeline->m_refCount > 0)
		{
			TRC_WARN("Can't unload a pipeline that is still in use");
			return;
		}

		pipeline->m_id = INVALID_ID;
		pipeline->~GPipeline();

	}
	bool PipelineManager::LoadDefaults()
	{

		trace::FileHandle vert_shad;
		trace::FileHandle frag_shad;

		std::string vert_src;
		std::string frag_src;

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.vert.glsl", trace::FileMode::READ, vert_shad))
		{
			TRC_ERROR("Failed to open file %s", "trace_core.shader.vert.glsl");
		}

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.frag.glsl", trace::FileMode::READ, frag_shad))
		{
			TRC_ERROR("Failed to open file %s", "trace_core.shader.frag.glsl");
		}

		trace::FileSystem::read_all_lines(vert_shad, vert_src);
		trace::FileSystem::read_all_lines(frag_shad, frag_src);

		std::cout << vert_src;
		std::cout << frag_src;

		trace::FileSystem::close_file(vert_shad);
		trace::FileSystem::close_file(frag_shad);

		GShader* VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		GShader* FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		ColorBlendState blend_state;
		blend_state.alpha_to_blend_coverage = true;

		DepthStencilState depth_stenc_state;
		depth_stenc_state.depth_test_enable = true;
		depth_stenc_state.maxDepth = 1.0f;
		depth_stenc_state.minDepth = 0.0f;
		depth_stenc_state.stencil_test_enable = false;

		InputLayout _layout = Vertex::get_input_layout();

		RaterizerState raterizer_state;
		raterizer_state.cull_mode = CullMode::BACK;
		raterizer_state.fill_mode = FillMode::SOLID;

		Viewport vp = {};
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = 800;
		vp.height = 600;
		vp.x = 0;
		vp.y = 0;


		Rect2D rect;
		rect.top = rect.left = 0;
		rect.right = 800;
		rect.bottom = 600;


		ShaderResourceBinding scene_data;
		scene_data.shader_stage = ShaderStage::VERTEX_SHADER;
		scene_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		scene_data.resource_size = sizeof(SceneGlobals);
		scene_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		scene_data.resource_name = "scene_globals";
		scene_data.count = 1;
		scene_data.index = 0;
		scene_data.slot = 0;

		ShaderResourceBinding scene_tex;
		scene_tex.shader_stage = ShaderStage::PIXEL_SHADER;
		scene_tex.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		scene_tex.resource_size = 0;
		scene_tex.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		scene_tex.resource_name = "testing";
		scene_tex.count = 3;
		scene_tex.index = 0;
		scene_tex.slot = 1;

		ShaderResourceBinding instance_data;
		instance_data.shader_stage = ShaderStage::PIXEL_SHADER;
		instance_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		instance_data.resource_size = sizeof(MaterialRenderData);
		instance_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		instance_data.resource_name = "instance_data";
		instance_data.count = 1;
		instance_data.index = 0;
		instance_data.slot = 0;

		ShaderResourceBinding local_data;
		local_data.shader_stage = ShaderStage::VERTEX_SHADER;
		local_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		local_data.resource_size = sizeof(glm::mat4);
		local_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		local_data.resource_name = "local_data";
		local_data.count = 1;
		local_data.index = 0;
		local_data.slot = 3;



		std::vector<ShaderResourceBinding> scene = {
			scene_data,
			scene_tex,
			instance_data,
			local_data
		};

		PipelineStateDesc desc;
		desc.blend_state = blend_state;
		desc.depth_sten_state = depth_stenc_state;
		desc.input_layout = _layout;
		desc.pixel_shader = FragShader;
		desc.rateriser_state = raterizer_state;
		desc.topology = PrimitiveTopology::TRIANGLE_LIST;
		desc.vertex_shader = VertShader;
		desc.view_port = vp;
		desc.resource_bindings_count = 4;
		desc.resource_bindings = scene;
		desc._renderPass = RENDERPASS::MAIN_PASS;
		desc.subpass_index = 0;

		if (CreatePipeline(desc, "standard_pipeline"))
		{
			standard_pipeline = { GetPipeline("standard_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this)};
			if (standard_pipeline->Initialize())
			{
				delete VertShader;
				delete FragShader;
			}
			else
			{
				TRC_ERROR("Failed to initialize default standard pipeline");
				return false;
			}
		}
		else
		{
			TRC_ERROR("Failed to load default standard pipeline");
			return false;
		}


		trace::FileHandle vert_shad0;
		trace::FileHandle frag_shad0;

		vert_src.clear();
		frag_src.clear();

		if (!trace::FileSystem::open_file("../assets/shaders/cubemap.vert.glsl", trace::FileMode::READ, vert_shad0))
		{
			TRC_ERROR("Failed to open file");
		}

		if (!trace::FileSystem::open_file("../assets/shaders/cubemap.frag.glsl", trace::FileMode::READ, frag_shad0))
		{
			TRC_ERROR("Failed to open file");
		}

		trace::FileSystem::read_all_lines(vert_shad0, vert_src);
		trace::FileSystem::read_all_lines(frag_shad0, frag_src);

		std::cout << vert_src;
		std::cout << frag_src;

		trace::FileSystem::close_file(vert_shad0);
		trace::FileSystem::close_file(frag_shad0);

		VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		raterizer_state.cull_mode = CullMode::FRONT;
		raterizer_state.fill_mode = FillMode::SOLID;


		ShaderResourceBinding scene_dat;
		scene_dat.shader_stage = ShaderStage::VERTEX_SHADER;
		scene_dat.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		scene_dat.resource_size = sizeof(SceneGlobals);
		scene_dat.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		scene_dat.resource_name = "scene_globals";
		scene_dat.count = 1;
		scene_dat.index = 0;
		scene_dat.slot = 0;

		ShaderResourceBinding cube_data;
		cube_data.shader_stage = ShaderStage::PIXEL_SHADER;
		cube_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		cube_data.resource_size = 0;
		cube_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		cube_data.resource_name = "CubeMap";
		cube_data.count = 1;
		cube_data.index = 0;
		cube_data.slot = 1;


		scene[0] = scene_dat;
		scene[1] = cube_data;

		desc.rateriser_state = raterizer_state;
		desc.resource_bindings_count = 2;
		desc.resource_bindings = scene;
		desc.vertex_shader = VertShader;
		desc.pixel_shader = FragShader;

		
		if (CreatePipeline(desc, "skybox_pipeline"))
		{
			skybox_pipeline = { GetPipeline("skybox_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this)};
			if (skybox_pipeline->Initialize())
			{
				delete VertShader;
				delete FragShader;
			}
			else
			{
				TRC_ERROR("Failed to initialize default skybox_pipeline");
				return false;
			}
		}
		else
		{
			TRC_ERROR("Failed to load default skybox_pipeline");
			return false;
		}

		return true;
	}
	PipelineManager* PipelineManager::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new PipelineManager();
		}

		return s_instance;
	}
	void PipelineManager::unloadDefault(GPipeline* pipeline)
	{
		if (pipeline->m_refCount > 0)
		{
			TRC_WARN("Can't unload a pipeline that is still in use");
			return;
		}

		pipeline->m_id = INVALID_ID;
		pipeline->~GPipeline();
	}
}