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
#include "reflection/SerializeTypes.h"


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
		emit << YAML::Key << "Pipeline Type" << YAML::Value << pipeline->GetPipelineType();


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

	bool PipelineSerializer::Serialize(Ref<GPipeline> pipeline, DataStream* stream)
	{
		std::string pipeline_name = pipeline->GetName();
		Reflection::Serialize(pipeline_name, stream, nullptr, Reflection::SerializationFormat::BINARY);

		uint32_t pip_type = pipeline->GetPipelineType();
		stream->Write<uint32_t>(pip_type);
		PipelineStateDesc ds = pipeline->GetDesc();
		Ref<GShader> vert = ShaderManager::get_instance()->GetShader(ds.vertex_shader->GetName());
		Ref<GShader> frag = ShaderManager::get_instance()->GetShader(ds.pixel_shader->GetName());
		uint64_t vertex_shader_id = GetUUIDFromName(ds.vertex_shader->GetName());
		uint64_t pixel_shader_id = GetUUIDFromName(ds.pixel_shader->GetName());
		stream->Write<uint64_t>(vertex_shader_id);
		stream->Write<uint64_t>(pixel_shader_id);

		uint32_t input_layout_stride = ds.input_layout.stride;
		stream->Write<uint32_t>(input_layout_stride);
		stream->Write<InputClassification>(ds.input_layout.input_class);
		int32_t input_layout_element_count = static_cast<int32_t>(ds.input_layout.elements.size());
		stream->Write<int32_t>(input_layout_element_count);
		stream->Write(ds.input_layout.elements.data(), input_layout_element_count * sizeof(InputLayout::Element));

		stream->Write<RasterizerState>(ds.rasteriser_state);

		stream->Write<DepthStencilState>(ds.depth_sten_state);

		stream->Write<ColorBlendState>(ds.blend_state);

		stream->Write<PRIMITIVETOPOLOGY>(ds.topology);

		stream->Write<Viewport>(ds.view_port);

		stream->Write<uint32_t>(ds.subpass_index);


		std::string pass_name = Renderer::get_instance()->GetRenderPassName(ds.render_pass);
		Reflection::Serialize(pass_name, stream, nullptr, Reflection::SerializationFormat::BINARY);


		return true;
	}

	/*
	* Pipeline
	*  '-> vertex_shader_id
	*  '-> fragment_shader_id
	*  '-> InputLayout stride
	*  '-> InputLayout class
	*  '-> InputLayout element_count
	*  '-> InputLayout elements
	*  '-> RasterizerState
	*  '-> DepthStencilState
	*  '-> ColorBlendState
	*  '-> PRIMITIVETOPOLOGY
	*  '-> Viewport
	*  '-> uint32_t subpass_index
	*  '-> pass_name_lenght
	*  '-> pass_name_data
	*/
	bool PipelineSerializer::Serialize(Ref<GPipeline> pipeline, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{
		if (!pipeline)
		{
			TRC_WARN("Pass in a valid pipeline, Function -> {}", __FUNCTION__);
			return false;
		}


		UUID id = GetUUIDFromName(pipeline->GetName());
		auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
			{
				return i.first == id;
			});

		if (it == map.end())
		{
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			PipelineStateDesc ds = pipeline->GetDesc();
			Ref<GShader> vert = ShaderManager::get_instance()->GetShader(ds.vertex_shader->GetName());
			Ref<GShader> frag = ShaderManager::get_instance()->GetShader(ds.pixel_shader->GetName());
			uint64_t vertex_shader_id = GetUUIDFromName(ds.vertex_shader->GetName());
			uint64_t pixel_shader_id = GetUUIDFromName(ds.pixel_shader->GetName());
			stream.Write<uint64_t>(vertex_shader_id);
			stream.Write<uint64_t>(pixel_shader_id);

			uint32_t input_layout_stride = ds.input_layout.stride;
			stream.Write<uint32_t>(input_layout_stride);
			stream.Write<InputClassification>(ds.input_layout.input_class);
			int32_t input_layout_element_count = static_cast<int32_t>(ds.input_layout.elements.size());
			stream.Write<int32_t>(input_layout_element_count);
			stream.Write(ds.input_layout.elements.data(), input_layout_element_count * sizeof(InputLayout::Element));

			stream.Write<RasterizerState>(ds.rasteriser_state);

			stream.Write<DepthStencilState>(ds.depth_sten_state);

			stream.Write<ColorBlendState>(ds.blend_state);

			stream.Write<PRIMITIVETOPOLOGY>(ds.topology);

			stream.Write<Viewport>(ds.view_port);

			stream.Write<uint32_t>(ds.subpass_index);


			std::string pass_name = Renderer::get_instance()->GetRenderPassName(ds.render_pass);
			int32_t pass_name_lenght = static_cast<int32_t>(pass_name.length() + 1);
			stream.Write<int32_t>(pass_name_lenght);
			stream.Write(pass_name.data(), pass_name_lenght);

			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.push_back(std::make_pair(id, ast_h));
		}

		return true;
	}

	/*
	* Shader
	*  '-> shader_stage
	*  '-> shader_size
	*  '-> shader_code
	*/
	bool PipelineSerializer::SerializeShader(Ref<GShader> shader, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{
		if (!shader)
		{
			TRC_WARN("Pass in a valid shader, Function -> {}", __FUNCTION__);
			return false;
		}


		UUID id = GetUUIDFromName(shader->GetName());
		auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
			{
				return i.first == id;
			});

		if (it == map.end())
		{
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			int shader_stage = shader->GetShaderStage();
			stream.Write<int>(shader_stage);
			uint32_t shader_size = static_cast<uint32_t>(shader->GetCode().size());
			stream.Write<uint32_t>(shader_size);
			stream.Write(shader->GetCode().data(), shader_size * sizeof(uint32_t));

			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.push_back(std::make_pair(id, ast_h));
		}

		return true;
	}

	bool PipelineSerializer::SerializeShader(Ref<GShader> shader, DataStream* stream)
	{
		std::string shader_name = shader->GetName();
		Reflection::Serialize(shader_name, stream, nullptr, Reflection::SerializationFormat::BINARY);

		int32_t data_index_count = static_cast<int32_t>(shader->GetDataIndex().size());
		stream->Write(data_index_count);

		for (auto& i : shader->GetDataIndex())
		{
			Reflection::Serialize(i.first, stream, nullptr, Reflection::SerializationFormat::BINARY);
			stream->Write(i.second);

		}

		int shader_stage = shader->GetShaderStage();
		stream->Write(shader_stage);
		int32_t shader_size = static_cast<int32_t>(shader->GetCode().size() * sizeof(uint32_t));
		stream->Write(shader_size);

		stream->Write(shader->GetCode().data(), shader_size);


		return true;
	}

	/*
	* Shader
	*  '-> data_index_count
	*    '-> name_length
	*    '-> name_data
	*    '-> index_data
	*  '-> shader_stage
	*  '-> shader_size
	*  '-> shader_code
	*/
	bool PipelineSerializer::SerializeShader(GShader* shader, const std::string& file_path)
	{
		FileStream stream(file_path, FileMode::WRITE);

		int32_t data_index_count = static_cast<int32_t>(shader->GetDataIndex().size());
		stream.Write(data_index_count);

		for (auto& i : shader->GetDataIndex())
		{
			int32_t name_length = static_cast<int32_t>(i.first.length() + 1);
			stream.Write(name_length);
			stream.Write(i.first.data(), name_length);

			stream.Write(i.second);

		}

		int shader_stage = shader->GetShaderStage();
		stream.Write(shader_stage);
		int32_t shader_size = static_cast<int32_t>(shader->GetCode().size() * sizeof(uint32_t));
		stream.Write(shader_size);

		stream.Write(shader->GetCode().data(), shader_size);

		return true;
	}

	bool PipelineSerializer::DeserializeShader(std::string& file_path, std::vector<uint32_t>& out_code, std::vector<std::pair<std::string, int>>& out_data_index)
	{
		FileStream stream(file_path, FileMode::READ);

		int data_index_count = 0;
		stream.Read(data_index_count);

		char buf[128] = { 0 };
		for (int i = 0; i < data_index_count; i++)
		{
			int name_length = 0;
			stream.Read(name_length);
			stream.Read(buf, name_length);
			std::string name = buf;
			int index_value = -1;
			stream.Read(index_value);

			out_data_index.push_back(std::make_pair(name, index_value));

		}

		int shader_stage = -1;
		stream.Read(shader_stage);

		int shader_size = 0;
		stream.Read(shader_size);

		out_code.resize(shader_size / sizeof(uint32_t));

		stream.Read(out_code.data(), shader_size);

		return true;
	}

	Ref<GShader> PipelineSerializer::DeserializeShader(DataStream* stream)
	{
		Ref<GShader> result;
		std::string shader_name;
		Reflection::Deserialize(shader_name, stream, nullptr, Reflection::SerializationFormat::BINARY);

		result = ShaderManager::get_instance()->GetShader(shader_name);
		if (result)
		{
			TRC_WARN("{} shader has already been loaded", shader_name);
			return result;
		}
		std::vector<uint32_t> code;
		std::vector<std::pair<std::string, int>> data_index;
		int32_t data_index_count = 0;
		stream->Read(data_index_count);

		for (int32_t i = 0; i < data_index_count; i++)
		{
			std::string name;
			Reflection::Deserialize(name, stream, nullptr, Reflection::SerializationFormat::BINARY);
			int index_value = -1;
			stream->Read(index_value);

			data_index.push_back(std::make_pair(name, index_value));

		}

		int shader_stage = -1;
		stream->Read(shader_stage);

		int shader_size = 0;
		stream->Read(shader_size);

		code.resize(shader_size / sizeof(uint32_t));

		stream->Read(code.data(), shader_size);

		result = ShaderManager::get_instance()->CreateShader(code, data_index, shader_name, (ShaderStage)shader_stage);

		return result;
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
		if (!data["Trace Version"] || !data["Pipeline Version"])
		{
			TRC_ERROR("These file is not a valid pipeline file {}", file_path);
			return result;
		}

		std::filesystem::path p = file_path;

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string pipeline_version = data["Pipeline Version"].as<std::string>(); // TODO: To be used later
		std::string pipeline_name = p.filename().string();

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
		if (result) result->SetPipelineType(type);

		if (!result)
		{
			TRC_ERROR("Unable to create pipeline -> name : {}, path : {}", pipeline_name, file_path);
		}

		return result;
	}

	Ref<GPipeline> PipelineSerializer::Deserialize(DataStream* stream)
	{
		std::string pipeline_name;
		Reflection::Deserialize(pipeline_name, stream, nullptr, Reflection::SerializationFormat::BINARY);

		Ref<GPipeline> result;

		result = PipelineManager::get_instance()->GetPipeline(pipeline_name);
		if (result)
		{
			TRC_WARN("{} has already been loaded", pipeline_name);
			return result;
		}

		uint32_t pip_type;
		stream->Read<uint32_t>(pip_type);

		UUID vert_id = 0;
		stream->Read<UUID>(vert_id);
		UUID frag_id = 0;
		stream->Read<UUID>(frag_id);
		PipelineStateDesc desc;
		desc.vertex_shader = ShaderManager::get_instance()->LoadShader_Runtime(vert_id).get();
		desc.pixel_shader = ShaderManager::get_instance()->LoadShader_Runtime(frag_id).get();

		stream->Read<uint32_t>(desc.input_layout.stride);
		stream->Read<InputClassification>(desc.input_layout.input_class);
		int input_layout_element_count = 0;
		stream->Read<int>(input_layout_element_count);
		desc.input_layout.elements.resize(input_layout_element_count);
		stream->Read(desc.input_layout.elements.data(), input_layout_element_count * sizeof(InputLayout::Element));

		stream->Read<RasterizerState>(desc.rasteriser_state);

		stream->Read<DepthStencilState>(desc.depth_sten_state);

		stream->Read<ColorBlendState>(desc.blend_state);

		stream->Read<PRIMITIVETOPOLOGY>(desc.topology);

		stream->Read<Viewport>(desc.view_port);

		stream->Read<uint32_t>(desc.subpass_index);


		std::string pass_name;
		Reflection::Deserialize(pass_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		desc.render_pass = Renderer::get_instance()->GetRenderPass(pass_name);

		ShaderResources s_res = {};
		ShaderParser::generate_shader_resources(desc.vertex_shader, s_res);
		ShaderParser::generate_shader_resources(desc.pixel_shader, s_res);
		desc.resources = s_res;

		result = PipelineManager::get_instance()->CreatePipeline(desc, pipeline_name, false);
		if (result)
		{
			result->SetPipelineType(pip_type);
		}

		return result;
	}



}