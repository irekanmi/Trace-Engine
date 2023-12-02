#include "pch.h"

#include "PipelineSerializer.h"
#include "core/FileSystem.h"
#include "resource/PipelineManager.h"
#include "resource/ShaderManager.h"
#include "render/ShaderParser.h"
#include "scene/UUID.h"
#include "render/GShader.h"
#include "backends/Renderutils.h"
#include "render/Renderer.h"


#include "yaml_util.h"

#include <string>


namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	

	bool PipelineSerializer::Serialize(Ref<GPipeline> pipeline, const std::string& file_path)
	{
		if (!pipeline)
		{
			TRC_WARN("Can't serialize an invalid pipeline");
			return false;
		}

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Pipeline Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Pipeline Name" << YAML::Value << pipeline->GetName();
		emit << YAML::Key << "Pipeline Type" << YAML::Value << pipeline->pipeline_type;


		PipelineStateDesc& desc = pipeline->GetDesc();
		emit << YAML::Key << "Vertex Shader" << YAML::Value << GetUUIDFromName(desc.vertex_shader->GetName());
		emit << YAML::Key << "Pixel Shader" << YAML::Value << GetUUIDFromName(desc.pixel_shader->GetName());

		// InputLayout
		{
			emit << YAML::Key << "InputLayout" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Stride" << YAML::Value << (int)desc.input_layout.stride;
			emit << YAML::Key << "Input Class" << YAML::Value << (int)desc.input_layout.input_class;
			emit << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;

			for (auto& i : desc.input_layout.elements)
			{
				emit << YAML::BeginMap;
				emit << YAML::Key << "Index" << YAML::Value << i.index;
				emit << YAML::Key << "Offset" << YAML::Value << i.offset;
				emit << YAML::Key << "Stride" << YAML::Value << i.stride;
				emit << YAML::Key << "Format" << YAML::Value << (int)i.format;

				emit << YAML::EndMap;
			}

			emit << YAML::EndSeq;


			emit << YAML::EndMap;
		};

		// RasterizerState
		{
			emit << YAML::Key << "RasterizerState" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Cull Mode" << YAML::Value << (int) desc.rasteriser_state.cull_mode;
			emit << YAML::Key << "Fill Mode" << YAML::Value << (int) desc.rasteriser_state.fill_mode;

			emit << YAML::EndMap;

		};

		// ColorBlendState
		{
			emit << YAML::Key << "ColorBlendState" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "alpha_to_blend_coverage" << YAML::Value << desc.blend_state.alpha_to_blend_coverage;
			emit << YAML::Key << "Src Color" << YAML::Value << (int)desc.blend_state.src_color;
			emit << YAML::Key << "Dst Color" << YAML::Value << (int)desc.blend_state.dst_color;
			emit << YAML::Key << "Color Op" << YAML::Value << (int)desc.blend_state.color_op;

			emit << YAML::Key << "Src Alpha" << YAML::Value << (int)desc.blend_state.src_alpha;
			emit << YAML::Key << "Dst Alpha" << YAML::Value << (int)desc.blend_state.dst_alpha;
			emit << YAML::Key << "Alpha Op" << YAML::Value << (int)desc.blend_state.alpha_op;

			emit << YAML::EndMap;
		};

		// DepthStencilState
		{
			emit << YAML::Key << "DepthStencilState" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "depth_test_enable" << YAML::Value << desc.depth_sten_state.depth_test_enable;
			emit << YAML::Key << "stencil_test_enable" << YAML::Value << desc.depth_sten_state.stencil_test_enable;
			emit << YAML::Key << "Min Depth" << YAML::Value << desc.depth_sten_state.minDepth;
			emit << YAML::Key << "Max Depth" << YAML::Value << desc.depth_sten_state.maxDepth;

			emit << YAML::EndMap;

		};

		emit << YAML::Key << "Topology" << YAML::Value << (int)desc.topology;


		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}

	Ref<GPipeline> PipelineSerializer::Deserialize(const std::string& file_path)
	{
		Ref<GPipeline> result;

		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return result;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Pipeline Version"] || !data["Pipeline Name"])
		{
			TRC_ERROR("These file is not a valid pipeline file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string pipeline_version = data["Pipeline Version"].as<std::string>(); // TODO: To be used later
		std::string pipeline_name = data["Pipeline Name"].as<std::string>();

		result = PipelineManager::get_instance()->GetPipeline(pipeline_name);
		if (result)
		{
			TRC_WARN("{} has already been loaded", pipeline_name);
			return result;
		}

		PipelineType type = (PipelineType)data["Pipeline Type"].as<int>();
		std::string vert_path = GetPathFromUUID(data["Vertex Shader"].as<uint64_t>()).string();
		std::string frag_path = GetPathFromUUID(data["Pixel Shader"].as<uint64_t>()).string();
				
		GShader* VertShader = ShaderManager::get_instance()->CreateShader_(vert_path, ShaderStage::VERTEX_SHADER).get();
		GShader* FragShader = ShaderManager::get_instance()->CreateShader_(frag_path, ShaderStage::PIXEL_SHADER).get();

		ShaderResources s_res = {};
		ShaderParser::generate_shader_resources(VertShader, s_res);
		ShaderParser::generate_shader_resources(FragShader, s_res);

		GRenderPass* pass = nullptr;
		if ((type & PipelineType::Surface_Material) != 0)
		{
			pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
		}

		PipelineStateDesc _ds2 = {};
		_ds2.vertex_shader = VertShader;
		_ds2.pixel_shader = FragShader;
		AutoFillPipelineDesc(
			_ds2,
			false
		);
		_ds2.render_pass = pass;
		_ds2.resources = s_res;

		InputLayout in_layout;
		{
			auto input = data["InputLayout"];
			in_layout.input_class = (InputClassification)input["Input Class"].as<int>();
			in_layout.stride = (uint32_t)input["Stride"].as<int>();

			for (auto i : input["Elements"])
			{
				InputLayout::Element elem;
				elem.index = (uint32_t)i["Index"].as<int>();
				elem.offset = (uint32_t)i["Offset"].as<int>();
				elem.stride = (uint32_t)i["Stride"].as<int>();
				elem.format = (Format)i["Format"].as<int>();
				in_layout.elements.push_back(elem);
			}
		};
		_ds2.input_layout = in_layout;

		RasterizerState rs;
		{
			auto _r = data["RasterizerState"];
			rs.cull_mode = (CullMode)_r["Cull Mode"].as<int>();
			rs.fill_mode = (FillMode)_r["Fill Mode"].as<int>();
		};
		_ds2.rasteriser_state = rs;

		ColorBlendState cbs;
		{
			auto _c = data["ColorBlendState"];
			cbs.alpha_to_blend_coverage = _c["alpha_to_blend_coverage"].as<bool>();
			cbs.alpha_op = (BlendOp)_c["Alpha Op"].as<int>();
			cbs.color_op = (BlendOp)_c["Color Op"].as<int>();
			cbs.dst_alpha = (BlendFactor)_c["Dst Alpha"].as<int>();
			cbs.dst_color = (BlendFactor)_c["Dst Color"].as<int>();
			cbs.src_alpha = (BlendFactor)_c["Src Alpha"].as<int>();
			cbs.src_color = (BlendFactor)_c["Src Color"].as<int>();
		}
		_ds2.blend_state = cbs;

		DepthStencilState dss;
		{
			auto _d = data["DepthStencilState"];
			dss.depth_test_enable = _d["depth_test_enable"].as<bool>();
			dss.stencil_test_enable = _d["stencil_test_enable"].as<bool>();
			dss.minDepth = _d["Min Depth"].as<float>();
			dss.maxDepth = _d["Max Depth"].as<float>();
		};
		_ds2.depth_sten_state = dss;

		_ds2.topology = (PRIMITIVETOPOLOGY)data["Topology"].as<int>();

		result = PipelineManager::get_instance()->CreatePipeline(_ds2, pipeline_name, false);

		if (!result)
		{
			TRC_ERROR("Unable to create pipeline -> name : {}, path : {}", pipeline_name, file_path);
		}

		return result;
	}

}