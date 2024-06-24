#include "pch.h"

#include "render/Material.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"
#include "backends/Renderutils.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"
#include "serialize/MemoryStream.h"
#include "serialize/MaterialSerializer.h"

namespace trace {


	extern std::string GetNameFromUUID(UUID uuid);

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
				
				TRC_TRACE("{} material was still in use ref count {} ", mat_instance.GetName(), mat_instance.m_refCount);
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
				mat_instance.GetMaterialData().clear();
				mat_instance.SetMaterialData(res);

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

	bool MaterialManager::RecreateMaterial(Ref<MaterialInstance> mat, Ref<GPipeline> pipeline)
	{
		RenderFunc::DestroyMaterial(mat.get());

		auto res = GetPipelineMaterialData(pipeline);
		mat->GetMaterialData().clear();
		mat->SetMaterialData(res);

		if (!RenderFunc::InitializeMaterial(
			mat.get(),
			pipeline
		))
		{
			TRC_WARN("Failed to initialize material {}", mat->GetName());
			mat->m_id = INVALID_ID;
			mat->GetRenderHandle()->m_internalData = nullptr;
			return false;
		}

		RenderFunc::PostInitializeMaterial(mat.get(), pipeline);

		return true;
	}

	void MaterialManager::Unload(MaterialInstance* material)
	{

		if (material->m_refCount > 0)
		{
			TRC_WARN("Can't unload a material that is still in use");
			return;
		}

		material->m_id = INVALID_ID;
		RenderFunc::DestroyMaterial(material);
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
			auto it1 = default_material->GetMaterialData().find("DIFFUSE_MAP");
			if (it1 != default_material->GetMaterialData().end())
			{
				it1->second.first = texture_manager->GetDefault("albedo_map");
			}
			auto it2 = default_material->GetMaterialData().find("SPECULAR_MAP");
			if (it2 != default_material->GetMaterialData().end())
			{
				it2->second.first = texture_manager->GetDefault("specular_map");
			}
			auto it3 = default_material->GetMaterialData().find("NORMAL_MAP");
			if (it3 != default_material->GetMaterialData().end())
			{
				it3->second.first = texture_manager->GetDefault("normal_map");
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
		default_material->m_path = "default";

		return true;
	}

	Ref<MaterialInstance> MaterialManager::LoadMaterial_Runtime(UUID id)
	{
		Ref<MaterialInstance> result;
		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		std::string name = GetNameFromUUID(id);
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

				std::string bin_dir;
				FindDirectory(AppSettings::exe_path, "Data/trmat.trbin", bin_dir);
				FileStream stream(bin_dir, FileMode::READ);

				stream.SetPosition(it->second.offset);
				char* data = new char[it->second.data_size];//TODO: Use custom allocator
				stream.Read(data, it->second.data_size);
				MemoryStream mem_stream(data, it->second.data_size);
				UUID pipe_id = 0;
				mem_stream.Read<UUID>(pipe_id);
				Ref<GPipeline> pipeline = PipelineManager::get_instance()->LoadPipeline_Runtime(pipe_id);
				auto res = GetPipelineMaterialData(pipeline);
				mat_instance.GetMaterialData().clear();
				mat_instance.SetMaterialData(res);

				MaterialSerializer::Deserialize(pipeline, &mat_instance, mem_stream);

				delete[] data;//TODO: Use custom allocator

				

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
				break;
			}
			i++;
		}


		result = { _mat, BIND_RENDER_COMMAND_FN(MaterialManager::Unload) };
		return result;


		return result;
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