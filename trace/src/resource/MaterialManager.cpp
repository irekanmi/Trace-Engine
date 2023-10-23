#include "pch.h"

#include "render/Material.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"
#include "backends/Renderutils.h"

//Temp
#include "serialize/MaterialSerializer.h"

namespace trace {

	MaterialManager* MaterialManager::s_instance = nullptr;


	MaterialManager::MaterialManager()
	{
	}

	MaterialManager::MaterialManager(uint32_t max_entries)
	{
	}

	MaterialManager::~MaterialManager()
	{
	}

	bool MaterialManager::Init(uint32_t max_entries)
	{
		m_numEntries = max_entries;
		m_hashtable.Init(m_numEntries);
		m_hashtable.Fill(INVALID_ID);

		m_materials.resize(m_numEntries);

		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_materials[i].m_id = INVALID_ID;
		}

		return true;
	}

	void MaterialManager::ShutDown()
	{
		if (!m_materials.empty())
		{

			for (MaterialInstance& mat_instance : m_materials)
			{
				if (mat_instance.m_id == INVALID_ID)
					continue;

				TRC_WARN("{} material was still in use ref count {} ", mat_instance.GetName(), mat_instance.m_refCount);
				mat_instance.~MaterialInstance();
			}
			m_materials.clear();
		}
	}


	Ref<MaterialInstance> MaterialManager::CreateMaterial(const std::string& name, Ref<GPipeline> pipeline)
	{
		Ref<MaterialInstance> result;
		MaterialInstance* _mat = nullptr;
		result = GetMaterial(name);
		if (result)
		{
			TRC_WARN("Material {} already exists", name);
			return result;
		}

		
		uint32_t i = 0;
		for (MaterialInstance& mat_instance : m_materials)
		{
			if (mat_instance.m_id == INVALID_ID)
			{
				auto res = GetPipelineMaterialData(pipeline);
				mat_instance.m_data.clear();
				mat_instance.m_data = std::move(res);

				if (!RenderFunc::InitializeMaterial(
					&mat_instance,
					pipeline
				))
				{
					TRC_WARN("Failed to initialize material {}", name);
					return result;
				}
				mat_instance.m_id = i;
				m_hashtable.Set(name, i);
				_mat = &mat_instance;
				_mat->m_path = name;
				//Temp ======
				_mat->m_refCount++;
				break;
			}
			i++;
		}

		

		result = { _mat, BIND_RENDER_COMMAND_FN(MaterialManager::Unload) };
		return result;
	}

	Ref<MaterialInstance> MaterialManager::GetMaterial(const std::string& name)
	{
		Ref<MaterialInstance> result;
		MaterialInstance* _mat = nullptr;
		uint32_t& hash = m_hashtable.Get_Ref(name);
		if (hash == INVALID_ID)
		{
			return result;
		}

		_mat = &m_materials[hash];
		if (_mat->m_id == INVALID_ID)
		{
			TRC_WARN("{} material has been destroyed", name);
			hash = INVALID_ID;
			return result;
		}
		result = { _mat, BIND_RENDER_COMMAND_FN(MaterialManager::Unload) };
		return result;
	}

	void MaterialManager::Unload(MaterialInstance* material)
	{

		if (material->m_refCount > 0)
		{
			TRC_WARN("Can't unload a material that is still in use");
			return;
		}

		material->m_id = INVALID_ID;
		material->~MaterialInstance();
	}

	bool MaterialManager::LoadDefaults()
	{
		TextureManager* texture_manager = TextureManager::get_instance();
		PipelineManager* pipeline_manager = PipelineManager::get_instance();

		Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");


		default_material  = CreateMaterial(
			"default",
			sp
		);

		//New Default
		{
			auto it1 = default_material->m_data.find("DIFFUSE_MAP");
			if (it1 != default_material->m_data.end())
			{
				it1->second.first = texture_manager->GetDefault("albedo_map");
			}
			auto it2 = default_material->m_data.find("SPECULAR_MAP");
			if (it2 != default_material->m_data.end())
			{
				it2->second.first = texture_manager->GetDefault("specular_map");
			}
			auto it3 = default_material->m_data.find("NORMAL_MAP");
			if (it3 != default_material->m_data.end())
			{
				it3->second.first = texture_manager->GetDefault("normal_map");
			}
			auto it4 = default_material->m_data.find("diffuse_color");
			if (it4 != default_material->m_data.end())
			{
				it4->second.first = glm::vec4(1.0f);
			}
			auto it5 = default_material->m_data.find("shininess");
			if (it5 != default_material->m_data.end())
			{
				it5->second.first = 32.0f;
			}
		};

		RenderFunc::PostInitializeMaterial(default_material.get(), sp);
		default_material->m_path = "default";

		return true;
	}

	MaterialManager* MaterialManager::get_instance()
	{

		if (s_instance == nullptr)
		{
			s_instance = new MaterialManager();
		}
		return s_instance;
	}

}