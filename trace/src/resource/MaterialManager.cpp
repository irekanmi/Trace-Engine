#include "pch.h"

#include "render/Material.h"
#include "MaterialManager.h"
#include "core/platform/Vulkan/VulkanMaterial.h"

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

		return true;
	}

	void MaterialManager::ShutDown()
	{
		if (m_materials)
		{
			switch (AppSettings::graphics_api)
			{
			case RenderAPI::Vulkan:
			{

				VulkanMaterial* materials = (VulkanMaterial*)m_materials;

				for (uint32_t i = 0; i < m_numEntries; i++)
				{
					if (materials[i].m_id != INVALID_ID)
					{
						materials[i].~VulkanMaterial();
						materials[i].m_id = INVALID_ID;
					}
				}

				break;
			}
			default:
			{
				TRC_ASSERT(false, "Render API Texture not suppoted");
				break;
			}
			}
			delete[] m_materials;
			m_materials = nullptr;
		}
	}


	bool MaterialManager::CreateMaterial(const std::string& name, Material material, Ref<GPipeline> pipeline)
	{

		if (m_hashtable.Get(name) != INVALID_ID)
		{
			TRC_WARN("Material {} already exists", name);
			return true;
		}

		switch (AppSettings::graphics_api)
		{
		case RenderAPI::Vulkan:
		{

			VulkanMaterial* materials = (VulkanMaterial*)m_materials;

			for (uint32_t i = 0; i < m_numEntries; i++)
			{
				if (materials[i].m_id == INVALID_ID)
				{
					VulkanMaterial* mat = &materials[i];
					new(mat) VulkanMaterial();
					if (!mat->Init(pipeline, material))
						return false;
					mat->m_id = i;
					m_hashtable.Set(name, i);
					break;
				}
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

	MaterialInstance* MaterialManager::GetMaterial(const std::string& name)
	{

		uint32_t hash = m_hashtable.Get(name);
		if (hash != INVALID_ID)
		{
			return (MaterialInstance*)(m_materials + (hash * m_matTypeSize));
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