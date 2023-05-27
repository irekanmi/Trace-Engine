#include "pch.h"

#include "render/Material.h"
#include "MaterialManager.h"
#include "render/Renderutils.h"
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
				mat_instance.~MaterialInstance();
			}
			m_materials.clear();
		}
	}


	bool MaterialManager::CreateMaterial(const std::string& name, Material material, Ref<GPipeline> pipeline)
	{

		if (m_hashtable.Get(name) != INVALID_ID)
		{
			TRC_WARN("Material {} already exists", name);
			return true;
		}

		uint32_t i = 0;
		for (MaterialInstance& mat_instance : m_materials)
		{
			if (mat_instance.m_id == INVALID_ID)
			{
				if (!RenderFunc::InitializeMaterial(
					&mat_instance,
					pipeline,
					material
				))
				{
					TRC_WARN("Failed to initialize material {}", name);
					return false;
				}
				mat_instance.m_id = i;
				m_hashtable.Set(name, i);
				break;
			}
			i++;
		}

		return true;
	}

	MaterialInstance* MaterialManager::GetMaterial(const std::string& name)
	{

		uint32_t hash = m_hashtable.Get(name);
		if (hash != INVALID_ID)
		{
			return &m_materials[hash];
		}

		TRC_WARN("Failed to create material {} please ensure material has been created", name);
		return nullptr;
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

	MaterialManager* MaterialManager::get_instance()
	{

		if (s_instance == nullptr)
		{
			s_instance = new MaterialManager();
		}
		return s_instance;
	}

}