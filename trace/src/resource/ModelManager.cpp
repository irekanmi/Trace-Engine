#include "pch.h"

#include "ModelManager.h"

namespace trace {

	ModelManager* ModelManager::s_instance = nullptr;

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
		}

	}
	Ref<Model> ModelManager::GetModel(const std::string& name)
	{
		Ref<Model> result;
		Model* _model = nullptr;
		uint32_t _id = m_hashtable.Get(name);
		if (_id == INVALID_ID)
		{
			TRC_ERROR("Please ensure model has been loaded => {}", name.c_str());
			return result;
		}

		_model = &m_models[_id];
		result = { _model,BIND_RENDER_COMMAND_FN(ModelManager::UnLoadModel) };
		return result;
	}
	Ref<Model> ModelManager::LoadModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		Ref<Model> result;
		Model* _model = nullptr;
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
		model->~Model();
	}
	ModelManager* ModelManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new ModelManager();
		}
		return s_instance;
	}
}