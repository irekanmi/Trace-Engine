#include "pch.h"

#include "ModelManager.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"

namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);


	ModelManager::ModelManager()
	{
	}
	ModelManager::~ModelManager()
	{
	}
	
	bool ModelManager::Init(uint32_t max_units)
	{
		m_numModelUnits = max_units;
		m_hashtable.Init(max_units);
		m_hashtable.Fill(INVALID_ID);

		m_models.resize(m_numModelUnits);

		return true;
	}
	void ModelManager::ShutDown()
	{
		for (Model& model : m_models)
		{
			if (model.m_id == INVALID_ID)
				continue;
			UnLoadModel(&model);
			TRC_TRACE("Model was still in use, name : {}, RefCount : {}", model.GetName(), model.m_refCount);
		}

	}
	Ref<Model> ModelManager::GetModel(const std::string& name)
	{
		Ref<Model> result;
		Model* _model = nullptr;
		uint32_t& _id = m_hashtable.Get_Ref(name);
		if (_id == INVALID_ID)
		{
			TRC_ERROR("Please ensure model has been loaded => {}", name.c_str());
			return result;
		}

		_model = &m_models[_id];
		if (_model->m_id == INVALID_ID)
		{
			TRC_WARN("{} model has been destroyed", name);
			_id = INVALID_ID;
			return result;
		}
		result = { _model,BIND_RENDER_COMMAND_FN(ModelManager::UnLoadModel) };
		return result;
	}
	Ref<Model> ModelManager::LoadModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		Ref<Model> result;
		Model* _model = nullptr;
		result = GetModel(name);
		if (result)
		{
			TRC_WARN("{} model has already been loaded", name);
			return result;
		}
		for (uint32_t i = 0; i < m_numModelUnits; i++)
		{
			if (m_models[i].m_id == INVALID_ID)
			{
				m_models[i].Init(vertices, indices);
				m_models[i].m_id = i;
				m_hashtable.Set(name, i);
				_model = &m_models[i];
				_model->m_path = name;
				break;
			}

		}

		result = { _model,BIND_RENDER_COMMAND_FN(ModelManager::UnLoadModel) };
		return result;
	}
	Ref<Model> ModelManager::LoadModel_(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<Model> result;
		Model* _model = nullptr;
		result = GetModel(name);
		if (result)
		{
			TRC_WARN("{} model has already been loaded", name);
			return result;
		}
		for (uint32_t i = 0; i < m_numModelUnits; i++)
		{
			if (m_models[i].m_id == INVALID_ID)
			{
				m_models[i].Init(vertices, indices);
				m_models[i].m_id = i;
				m_hashtable.Set(name, i);
				_model = &m_models[i];
				_model->m_path = p;
				break;
			}

		}

		result = { _model,BIND_RENDER_COMMAND_FN(ModelManager::UnLoadModel) };
		return result;
	}
	void ModelManager::UnLoadModel(Model* model)
	{

		if (!model)
			return;

		if (model->m_refCount > 0)
		{
			TRC_WARN("Can't unload a model that is till in use");
			return;
		}

		model->m_id = INVALID_ID;
		model->Release();
	}
	Ref<Model> ModelManager::LoadModel_Runtime(UUID id)
	{
		std::string name = GetNameFromUUID(id);
		Ref<Model> result;
		Model* _model = nullptr;

		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		result = GetModel(name);
		if (result)
		{
			TRC_WARN("{} model has already been loaded", name);
			return result;
		}
		for (uint32_t i = 0; i < m_numModelUnits; i++)
		{
			if (m_models[i].m_id == INVALID_ID)
			{
				std::string bin_dir;
				FindDirectory(AppSettings::exe_path, "Data/trmdl.trbin", bin_dir);
				FileStream stream(bin_dir, FileMode::READ);

				stream.SetPosition(it->second.offset);

				int vertex_count = 0;
				stream.Read<int>(vertex_count);
				std::vector<Vertex> vertices;
				vertices.resize(vertex_count);
				stream.Read(vertices.data(), vertex_count * sizeof(Vertex));

				int index_count = 0;
				stream.Read<int>(index_count);
				std::vector<uint32_t> indices;
				indices.resize(index_count);
				stream.Read(indices.data(), index_count * sizeof(uint32_t));
				

				m_models[i].Init(vertices, indices);
				m_models[i].m_id = i;
				m_hashtable.Set(name, i);
				_model = &m_models[i];
				break;
			}

		}

		result = { _model,BIND_RENDER_COMMAND_FN(ModelManager::UnLoadModel) };
		return result;
	}
	ModelManager* ModelManager::get_instance()
	{
		static ModelManager* s_instance = new ModelManager;
		return s_instance;
	}
}