#include "pch.h"

#include "DefaultAssetsManager.h"
#include "backends/Renderutils.h"
#include "resource/GenericAssetManager.h"
#include "render/Renderer.h"
#include "render/ShaderParser.h"
#include "render/Material.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "core/Utils.h"


namespace trace {

	Ref<MaterialInstance> DefaultAssetsManager::default_material = Ref<MaterialInstance>();
	Ref<Model> DefaultAssetsManager::Cube = Ref<Model>();
	Ref<Model> DefaultAssetsManager::Sphere = Ref<Model>();
	Ref<Model> DefaultAssetsManager::Plane = Ref<Model>();
	Ref<GPipeline> DefaultAssetsManager::skybox_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::light_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::gbuffer_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::skinned_gbuffer_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::text_batch_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::text_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::quad_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::debug_line_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::particle_billboard_pipeline = Ref<GPipeline>();
	Ref<GPipeline> DefaultAssetsManager::particle_velocity_aligned_pipeline = Ref<GPipeline>();
	Texture_Ref DefaultAssetsManager::default_diffuse_map = Ref<GTexture>();
	Texture_Ref DefaultAssetsManager::default_specular_map = Ref<GTexture>();
	Texture_Ref DefaultAssetsManager::default_normal_map = Ref<GTexture>();
	Texture_Ref DefaultAssetsManager::black_texture = Ref<GTexture>();
	Texture_Ref DefaultAssetsManager::transparent_texture = Ref<GTexture>();

	std::string DefaultAssetsManager::assets_path = "";

	bool DefaultAssetsManager::LoadAssets()
	{
		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		

		//Textures
		{
			uint32_t dimension = 256;
			uint32_t channels = 4;

			unsigned char* pixel = new unsigned char[16];
			pixel[0] = 255;
			pixel[1] = 255;
			pixel[2] = 255;
			pixel[3] = 255;
			//memset(pixel, 255, dimension * dimension * channels);
			//for (uint32_t row = 0; row < dimension; row++)
			//{
			//	for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
			//	{
			//		uint32_t index = (row * dimension) + coloumn;
			//		uint32_t _idx = index * channels;
			//		if (row % 2)
			//		{
			//			if (coloumn % 2)
			//			{
			//				pixel[_idx + 0] = 255;//0;
			//				pixel[_idx + 1] = 255;//0;
			//			}
			//		}
			//		else
			//		{
			//			if (!(coloumn % 2))
			//			{
			//				pixel[_idx + 0] = 255;//0;
			//				pixel[_idx + 1] = 255;//0;
			//			}
			//		}
			//	}
			//}

			TextureDesc texture_desc;
			texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
			texture_desc.m_channels = 4;
			texture_desc.m_format = Format::R8G8B8A8_UNORM;
			texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
			texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
			texture_desc.m_usage = UsageFlag::DEFAULT;
			texture_desc.m_width = (uint32_t)1;
			texture_desc.m_height = (uint32_t)1;
			texture_desc.m_image_type = ImageType::IMAGE_2D;
			texture_desc.m_data.push_back(pixel);
			texture_desc.m_numLayers = 1;
			texture_desc.m_mipLevels = 1;

			default_diffuse_map = asset_manager->CreateAssetHandle<GTexture>("albedo_map", texture_desc);
			

			dimension = 16;
			channels = 4;



			pixel = new unsigned char[16];
			pixel[0] = 255;
			pixel[1] = 255;
			pixel[2] = 255;
			pixel[3] = 255;
			//memset(pixel, 0, dimension * dimension * channels);
			texture_desc.m_width = (uint32_t)1;
			texture_desc.m_height = (uint32_t)1;
			texture_desc.m_channels = 4;
			texture_desc.m_data[0] = (pixel);
			texture_desc.m_mipLevels = 1;


			/*for (uint32_t row = 0; row < dimension; row++)
			{
				for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
				{
					uint32_t index = (row * dimension) + coloumn;
					uint32_t _idx = index * channels;
					pixel[_idx + 0] = 255;
					pixel[_idx + 1] = 255;
					pixel[_idx + 2] = 255;
					pixel[_idx + 3] = 255;
				}
			}*/

			default_specular_map = asset_manager->CreateAssetHandle<GTexture>("specular_map", texture_desc);
			


			dimension = 16;
			channels = 4;



			pixel = new unsigned char[16];
			pixel[0] = 128;
			pixel[1] = 128;
			pixel[2] = 255;
			pixel[3] = 255;
			//memset(pixel, 0, dimension * dimension * channels);
			texture_desc.m_width = (uint32_t)1;
			texture_desc.m_height = (uint32_t)1;
			texture_desc.m_channels = 4;
			texture_desc.m_data[0] = pixel;
			texture_desc.m_format = Format::R8G8B8A8_UNORM;
			texture_desc.m_mipLevels = 1;


			/*for (uint32_t row = 0; row < dimension; row++)
			{
				for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
				{
					uint32_t index = (row * dimension) + coloumn;
					uint32_t _idx = index * channels;
					pixel[_idx + 0] = 128;
					pixel[_idx + 1] = 128;
					pixel[_idx + 2] = 255;
					pixel[_idx + 3] = 255;
				}
			}*/

			default_normal_map = asset_manager->CreateAssetHandle<GTexture>("normal_map", texture_desc);
			

			unsigned char* black_texture_data = new unsigned char[16];
			black_texture_data[0] = 0;
			black_texture_data[1] = 0;
			black_texture_data[2] = 0;
			black_texture_data[3] = 255;
			texture_desc.m_width = 1;
			texture_desc.m_height = 1;
			texture_desc.m_data[0] = black_texture_data;
			texture_desc.m_format = Format::R8G8B8A8_UNORM;
			texture_desc.m_mipLevels = 1;

			black_texture = asset_manager->CreateAssetHandle<GTexture>("black_texture", texture_desc);
			

			unsigned char* transparent_texture_data = new unsigned char[16];
			transparent_texture_data[0] = 0;
			transparent_texture_data[1] = 0;
			transparent_texture_data[2] = 0;
			transparent_texture_data[3] = 0;
			texture_desc.m_width = 1;
			texture_desc.m_height = 1;
			texture_desc.m_data[0] = transparent_texture_data;
			texture_desc.m_format = Format::R8G8B8A8_UNORM;
			texture_desc.m_mipLevels = 1;

			transparent_texture = asset_manager->CreateAssetHandle<GTexture>("transparent_texture", texture_desc);
			
		};

		//Pipelines
		{
			GShader* VertShader;
			GShader* FragShader;

			{
				RasterizerState raterizer_state;
				raterizer_state.cull_mode = CullMode::FRONT;
				raterizer_state.fill_mode = FillMode::SOLID;

				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("cubemap.vert.glsl", "cubemap.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("cubemap.frag.glsl", "cubemap.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds1 = {};
				_ds1.rasteriser_state = raterizer_state;
				_ds1.vertex_shader = VertShader;
				_ds1.pixel_shader = FragShader;
				_ds1.resources = s_res;

				AutoFillPipelineDesc(
					_ds1,
					true,
					false
				);
				_ds1.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");
				skybox_pipeline = asset_manager->CreateAssetHandle<GPipeline>("skybox_pipeline", _ds1);
				
				if (!skybox_pipeline)
				{
					TRC_ERROR("Failed to initialize or create default skybox_pipeline");
					return false;
				}
			}

			{

				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("lights.vert.glsl", "lights.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("lights.frag.glsl", "lights.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);


				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = Vertex::get_input_layout();


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");

				light_pipeline = asset_manager->CreateAssetHandle<GPipeline>("light_pipeline", _ds2);
				
				if (!light_pipeline)
				{
					TRC_ERROR("Failed to initialize or create light_pipeline");
					return false;
				}

			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("quad.vert.glsl", "quad.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("quad.frag.glsl", "quad.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.input_layout = Vertex::get_input_layout();
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
				Enable_WeightedOIT(_ds2);
				_ds2.depth_sten_state = { true, false, false, 1.0f, 0.0f };
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };


				quad_pipeline = asset_manager->CreateAssetHandle<GPipeline>("quad_batch_pipeline", _ds2);
				
				if (!quad_pipeline)
				{
					TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
					return false;
				}
			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("text.vert.glsl", "text.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("text_MSDF.frag.glsl", "text_MSDF.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };
				Enable_Blending(_ds2);
				_ds2.depth_sten_state = { true, true, true, 0.0f, 1.0f };

				text_batch_pipeline = asset_manager->CreateAssetHandle<GPipeline>("text_batch_pipeline", _ds2);
				
				if (!text_batch_pipeline)
				{
					TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
					return false;
				}
			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("text_v.vert.glsl", "text_v.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("text_f.frag.glsl", "text_f.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.input_layout = Vertex::get_input_layout();
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
				Enable_WeightedOIT(_ds2);
				_ds2.depth_sten_state = { true, false, false, 1.0f, 0.0f };
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };

				text_pipeline = asset_manager->CreateAssetHandle<GPipeline>("text_pipeline", _ds2);
				
				if (!text_pipeline)
				{
					TRC_ERROR("Failed to initialize or create text_pipeline");
					return false;
				}
			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("particle_billboard.vert.glsl", "particle_billboard.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("particle_billboard.frag.glsl", "particle_billboard.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.input_layout = Vertex::get_input_layout();
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
				Enable_WeightedOIT(_ds2);
				_ds2.depth_sten_state = { true, false, false, 1.0f, 0.0f };
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };


				particle_billboard_pipeline = asset_manager->CreateAssetHandle<GPipeline>("particle_billboard_pipeline", _ds2);

				if (!particle_billboard_pipeline)
				{
					TRC_ERROR("Failed to initialize or create particle_billboard_pipeline");
					return false;
				}
			};
			
			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("particle_velocity_aligned.vert.glsl", "particle_velocity_aligned.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("particle_billboard.frag.glsl", "particle_billboard.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.input_layout = Vertex::get_input_layout();
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
				Enable_WeightedOIT(_ds2);
				_ds2.depth_sten_state = { true, false, false, 1.0f, 0.0f };
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };


				particle_velocity_aligned_pipeline = asset_manager->CreateAssetHandle<GPipeline>("particle_velocity_aligned_pipeline", _ds2);

				if (!particle_velocity_aligned_pipeline)
				{
					TRC_ERROR("Failed to initialize or create particle_velocity_aligned_pipeline");
					return false;
				}
			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("debug_line.vert.glsl", "debug_line.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("debug_line.frag.glsl", "debug_line.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader;
				_ds2.pixel_shader = FragShader;
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS"); // TODO: Create custom debug pass
				_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };
				_ds2.topology = PRIMITIVETOPOLOGY::LINE_LIST;
				Enable_Blending(_ds2);
				_ds2.depth_sten_state = { true, true, false, 0.0f, 1.0f };

				debug_line_pipeline = asset_manager->CreateAssetHandle<GPipeline>("debug_line_pipeline", _ds2);
				
				if (!debug_line_pipeline)
				{
					TRC_ERROR("Failed to initialize or create debug_line_pipeline");
					return false;
				}
			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("trace_core.shader.vert.glsl", "trace_core.shader.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("parallelex_brdf.frag.glsl", "parallelex_brdf.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds;
				_ds.vertex_shader = VertShader;
				_ds.pixel_shader = FragShader;
				_ds.resources = s_res;

				AutoFillPipelineDesc(
					_ds
				);
				_ds.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
				_ds.blend_state.alpha_to_blend_coverage = false;

				gbuffer_pipeline = asset_manager->CreateAssetHandle<GPipeline>("gbuffer_pipeline", _ds);
				
				if (!gbuffer_pipeline)
				{
					TRC_ERROR("Failed to initialize or create default gbuffer pipeline");
					return false;
				}
				gbuffer_pipeline->SetPipelineType(PipelineType::Surface_Material);


			};

			{
				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("skinned_model.vert.glsl", "skinned_model.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("parallelex_brdf.frag.glsl", "parallelex_brdf.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader.get();
				FragShader = frag_shader.get();

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds;
				_ds.vertex_shader = VertShader;
				_ds.pixel_shader = FragShader;
				_ds.resources = s_res;

				AutoFillPipelineDesc(
					_ds, false
				);
				_ds.input_layout = SkinnedVertex::get_input_layout();
				_ds.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
				_ds.blend_state.alpha_to_blend_coverage = false;

				skinned_gbuffer_pipeline = asset_manager->CreateAssetHandle<GPipeline>("skinned_gbuffer_pipeline", _ds);
				
				if (!skinned_gbuffer_pipeline)
				{
					TRC_ERROR("Failed to initialize or create default skinned gbuffer pipeline");
					return false;
				}
				skinned_gbuffer_pipeline->SetPipelineType(PipelineType::Surface_Material);


			};
		};

		// Materials
		{
			Ref<GPipeline> sp = asset_manager->Get<GPipeline>("gbuffer_pipeline");
			default_material = asset_manager->CreateAssetHandle<MaterialInstance>("default", sp);

			//New Default
			{
				auto it1 = default_material->GetMaterialData().find("DIFFUSE_MAP");
				if (it1 != default_material->GetMaterialData().end())
				{
					it1->second.first = asset_manager->Get<GTexture>("albedo_map");
				}
				auto it2 = default_material->GetMaterialData().find("SPECULAR_MAP");
				if (it2 != default_material->GetMaterialData().end())
				{
					it2->second.first = asset_manager->Get<GTexture>("specular_map");
				}
				auto it3 = default_material->GetMaterialData().find("NORMAL_MAP");
				if (it3 != default_material->GetMaterialData().end())
				{
					it3->second.first = asset_manager->Get<GTexture>("normal_map");
				}
				auto it4 = default_material->GetMaterialData().find("diffuse_color");
				if (it4 != default_material->GetMaterialData().end())
				{
					it4->second.first = glm::vec4(1.0f);
				}
				auto it5 = default_material->GetMaterialData().find("shininess");
				if (it5 != default_material->GetMaterialData().end())
				{
					it5->second.first = 32.0f;
				}
			};
			RenderFunc::PostInitializeMaterial(default_material.get(), sp);
		};

		//Models
		{
			std::vector<Vertex> verts;
			std::vector<uint32_t> _ind;

			//Cube
			{
				generateDefaultCube(verts, _ind);
				generateVertexTangent(verts, _ind);
				Cube = asset_manager->CreateAssetHandle<Model>("Cube", verts, _ind);
				
			}

			// Sphere
			{
				verts.clear();
				_ind.clear();

				generateSphere(verts, _ind, 1.0f, 45, 45);
				generateVertexTangent(verts, _ind);
				Sphere = asset_manager->CreateAssetHandle<Model>("Sphere", verts, _ind);
				

			};

			//Plane
			{
				verts.clear();
				_ind.clear();

				generateDefaultPlane(verts, _ind);
				generateVertexTangent(verts, _ind);
				Plane = asset_manager->CreateAssetHandle<Model>("Plane", verts, _ind);
				
			};
		};



		return true;
	}

	bool DefaultAssetsManager::LoadAssets_Runtime()
	{
		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();


		UUID id = GetUUIDFromName("skybox_pipeline");
		skybox_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("light_pipeline");
		light_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("quad_batch_pipeline");
		quad_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("text_batch_pipeline");
		text_batch_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("text_pipeline");
		text_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("debug_line_pipeline");
		debug_line_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

		id = GetUUIDFromName("gbuffer_pipeline");
		gbuffer_pipeline = asset_manager->Load_Runtime<GPipeline>(id);


		id = GetUUIDFromName("Cube");
		Cube = asset_manager->Load_Runtime<Model>(id);

		id = GetUUIDFromName("Sphere");
		Sphere = asset_manager->Load_Runtime<Model>(id);

		id = GetUUIDFromName("Plane");
		Plane = asset_manager->Load_Runtime<Model>(id);

		return true;
	}

	void DefaultAssetsManager::LoadAssetsPath()
	{
		FindDirectory(AppSettings::exe_path, "assets/", assets_path);
	}

	void DefaultAssetsManager::ReleaseAssets()
	{
		//Temp -------------
		default_material->Decrement();
		// ---------
		default_material.free();
		Cube.free();
		Sphere.free();
		Plane.free();
		skybox_pipeline.free();
		light_pipeline.free();
		gbuffer_pipeline.free();
		skinned_gbuffer_pipeline.free();
		text_batch_pipeline.free();
		text_pipeline.free();
		quad_pipeline.free();
		debug_line_pipeline.free();
		particle_billboard_pipeline.free();
		default_diffuse_map.free();
		default_specular_map.free();
		default_normal_map.free();
		black_texture.free();
		transparent_texture.free();
	}
}
