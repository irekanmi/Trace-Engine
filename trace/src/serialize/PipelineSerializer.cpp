#include "pch.h"

#include "PipelineSerializer.h"
#include "core/FileSystem.h"

#include "resource/GenericAssetManager.h"
#include "render/ShaderParser.h"
#include "scene/UUID.h"
#include "render/GShader.h"
#include "backends/Renderutils.h"
#include "render/Renderer.h"
#include "reflection/SerializeTypes.h"
#include "shader_graph/ShaderGraph.h"
#include "serialize/GenericSerializer.h"


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
		emit << YAML::Key << "Material Type" << YAML::Value << (int)pipeline->GetType();
		if (pipeline->GetShaderGraph())
		{
			emit << YAML::Key << "Shader Graph" << YAML::Value << pipeline->GetShaderGraph()->GetUUID();
		}
		else
		{



			PipelineStateDesc& desc = pipeline->GetDesc();

			emit << YAML::Key << "Vertex Shader" << YAML::Value << desc.vertex_shader->GetUUID();
			emit << YAML::Key << "Pixel Shader" << YAML::Value << desc.pixel_shader->GetUUID();

			// InputLayout
			{
				emit << YAML::Key << "InputLayout" << YAML::Value;
				Reflection::Serialize(desc.input_layout, &emit, nullptr, Reflection::SerializationFormat::YAML);
			};

			// RasterizerState
			{
				emit << YAML::Key << "RasterizerState" << YAML::Value;
				Reflection::Serialize(desc.rasteriser_state, &emit, nullptr, Reflection::SerializationFormat::YAML);

			};

			// ColorBlendState
			{
				emit << YAML::Key << "ColorBlendState" << YAML::Value;
				Reflection::Serialize(desc.blend_state, &emit, nullptr, Reflection::SerializationFormat::YAML);
			};

			// DepthStencilState
			{
				emit << YAML::Key << "DepthStencilState" << YAML::Value;
				Reflection::Serialize(desc.depth_sten_state, &emit, nullptr, Reflection::SerializationFormat::YAML);

			};

			emit << YAML::Key << "Topology" << YAML::Value << (int)desc.topology;

		}
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
		bool has_shader_graph = false;
		/*if (pipeline->GetShaderGraph())
		{
			has_shader_graph = true;
			stream->Write(has_shader_graph);
			stream->Write(pipeline->GetShaderGraph()->GetUUID());

			return true;
		}*/
		stream->Write(has_shader_graph);
		PipelineStateDesc ds = pipeline->GetDesc();

		uint8_t pip_type = (uint8_t)pipeline->GetType();
		stream->Write<uint8_t>(pip_type);

		stream->Write(ds.vertex_shader->GetUUID());
		stream->Write(ds.pixel_shader->GetUUID());

		// InputLayout
		{
			Reflection::Serialize(ds.input_layout, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		// RasterizerState
		{
			Reflection::Serialize(ds.rasteriser_state, stream, nullptr, Reflection::SerializationFormat::BINARY);

		};

		// ColorBlendState
		{
			Reflection::Serialize(ds.blend_state, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		// DepthStencilState
		{
			Reflection::Serialize(ds.depth_sten_state, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		Reflection::Serialize(ds.topology, stream, nullptr, Reflection::SerializationFormat::BINARY);

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
			Ref<GShader> vert = GenericAssetManager::get_instance()->Get<GShader>(ds.vertex_shader->GetName());
			Ref<GShader> frag = GenericAssetManager::get_instance()->Get<GShader>(ds.pixel_shader->GetName());
			uint64_t vertex_shader_id = GetUUIDFromName(ds.vertex_shader->GetName());
			uint64_t pixel_shader_id = GetUUIDFromName(ds.pixel_shader->GetName());
			stream.Write<uint64_t>(vertex_shader_id);
			stream.Write<uint64_t>(pixel_shader_id);

			uint32_t input_layout_stride = ds.input_layout.stride;
			stream.Write<uint32_t>(input_layout_stride);
			stream.Write<InputClassification>(ds.input_layout.input_class);
			int32_t input_layout_element_count = static_cast<int32_t>(ds.input_layout.elements.size());
			stream.Write<int32_t>(input_layout_element_count);
			stream.Write(ds.input_layout.elements.data(), input_layout_element_count * sizeof(Element));

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

		result = GenericAssetManager::get_instance()->TryGet<GShader>(shader_name);
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

		result = GenericAssetManager::get_instance()->CreateAssetHandle<GShader>(shader_name, code, data_index, (ShaderStage)shader_stage);

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

		result = GenericAssetManager::get_instance()->TryGet<GPipeline>(pipeline_name);
		if (result)
		{
			TRC_WARN("{} has already been loaded", pipeline_name);
			return result;
		}

		if (data["Shader Graph"])
		{
			std::string shader_graph_path = GetPathFromUUID(data["Shader Graph"].as<uint64_t>()).string();
			Ref<ShaderGraph> shader_graph = GenericSerializer::Deserialize<ShaderGraph>(shader_graph_path);
			if (!shader_graph)
			{
				return Ref<GPipeline>();
			}

			return shader_graph->GetPipeline();
		}

		PipelineType type = (PipelineType)data["Pipeline Type"].as<int>();
		MaterialType material_type = MaterialType::OPAQUE_LIT;
		if (data["Material Type"])
		{
			material_type = (MaterialType)data["Material Type"].as<int>();
		}

		std::string vert_path = GetPathFromUUID(data["Vertex Shader"].as<uint64_t>()).string();
		std::string frag_path = GetPathFromUUID(data["Pixel Shader"].as<uint64_t>()).string();
				
		GShader* VertShader = GenericAssetManager::get_instance()->CreateAssetHandle_<GShader>(vert_path, vert_path, ShaderStage::VERTEX_SHADER).get();
		GShader* FragShader = GenericAssetManager::get_instance()->CreateAssetHandle_<GShader>(frag_path, frag_path, ShaderStage::PIXEL_SHADER).get();

		ShaderResources s_res = {};
		ShaderParser::generate_shader_resources(VertShader, s_res);
		ShaderParser::generate_shader_resources(FragShader, s_res);

		GRenderPass* pass = nullptr;
		switch (material_type)
		{
		case MaterialType::OPAQUE_LIT:
		{
			pass = (GRenderPass*)Renderer::get_instance()->GetAvaliableRenderPasses()["GBUFFER_PASS"];
			break;
		}
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
			Reflection::Deserialize(in_layout, &input, nullptr, Reflection::SerializationFormat::YAML);
		};
		_ds2.input_layout = in_layout;

		RasterizerState rs;
		{
			auto _r = data["RasterizerState"];
			Reflection::Deserialize(rs, &_r, nullptr, Reflection::SerializationFormat::YAML);
		};
		_ds2.rasteriser_state = rs;

		ColorBlendState cbs;
		{
			auto _c = data["ColorBlendState"];
			Reflection::Deserialize(cbs, &_c, nullptr, Reflection::SerializationFormat::YAML);
		}
		_ds2.blend_state = cbs;

		DepthStencilState dss;
		{
			auto _d = data["DepthStencilState"];
			Reflection::Deserialize(dss, &_d, nullptr, Reflection::SerializationFormat::YAML);;
		};
		_ds2.depth_sten_state = dss;

		_ds2.topology = (PRIMITIVETOPOLOGY)data["Topology"].as<int>();

		result = GenericAssetManager::get_instance()->CreateAssetHandle<GPipeline>(pipeline_name, _ds2, false);
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

		result = GenericAssetManager::get_instance()->TryGet<GPipeline>(pipeline_name);
		if (result)
		{
			TRC_WARN("{} has already been loaded", pipeline_name);
			return result;
		}

		bool has_shader_graph = false;
		stream->Read(has_shader_graph);
		if (has_shader_graph)
		{
			UUID shader_graph_id = 0;
			stream->Read(shader_graph_id);
			Ref<ShaderGraph> shader_graph = GenericAssetManager::get_instance()->Load_Runtime<ShaderGraph>(shader_graph_id);
			if (shader_graph)
			{
				return shader_graph->GetPipeline();
			}
			return result;
		}

		uint8_t pip_type;
		stream->Read<uint8_t>(pip_type);
		MaterialType mat_type = (MaterialType)pip_type;

		

		UUID vert_id = 0;
		stream->Read<UUID>(vert_id);
		UUID frag_id = 0;
		stream->Read<UUID>(frag_id);
		PipelineStateDesc desc;
		desc.vertex_shader = GenericAssetManager::get_instance()->Load_Runtime<GShader>(vert_id).get();
		desc.pixel_shader = GenericAssetManager::get_instance()->Load_Runtime<GShader>(frag_id).get();

		// InputLayout
		{
			Reflection::Deserialize(desc.input_layout, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		// RasterizerState
		{
			Reflection::Deserialize(desc.rasteriser_state, stream, nullptr, Reflection::SerializationFormat::BINARY);

		};

		// ColorBlendState
		{
			Reflection::Deserialize(desc.blend_state, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		// DepthStencilState
		{
			Reflection::Deserialize(desc.depth_sten_state, stream, nullptr, Reflection::SerializationFormat::BINARY);
		};

		Reflection::Deserialize(desc.topology, stream, nullptr, Reflection::SerializationFormat::BINARY);



		std::string pass_name;
		Reflection::Deserialize(pass_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		desc.render_pass = Renderer::get_instance()->GetRenderPass(pass_name);

		ShaderResources s_res = {};
		ShaderParser::generate_shader_resources(desc.vertex_shader, s_res);
		ShaderParser::generate_shader_resources(desc.pixel_shader, s_res);
		desc.resources = s_res;

		result = GenericAssetManager::get_instance()->CreateAssetHandle<GPipeline>(pipeline_name, desc, false);
		if (result)
		{
			result->SetType(mat_type);
		}

		return result;
	}



}