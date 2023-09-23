#include "pch.h"

#include "ResourceSystem.h"
#include "core/Enums.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "MeshManager.h"
#include "PipelineManager.h"
#include "MaterialManager.h"
#include "ShaderManager.h"
#include "FontManager.h"

namespace trace {

	ResourceSystem* ResourceSystem::s_instance = nullptr;

	ResourceSystem::ResourceSystem()
	{
	}
	ResourceSystem::~ResourceSystem()
	{
	}
	bool ResourceSystem::Init()
	{
		bool result = false;

		m_textureManager = TextureManager::get_instance();
		m_modelManager = ModelManager::get_instance();
		m_meshManager = MeshManager::get_instance();
		m_pipelineManager = PipelineManager::get_instance();
		m_materialManager = MaterialManager::get_instance();
		m_shaderManager = ShaderManager::get_instance();
		m_fontManager = FontManager::get_instance();

		//TODO: Configurable
		result = m_textureManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialize texture manager");

		result = m_fontManager->Init(256);
		TRC_ASSERT(result, "Failed to initialized shader manager");

		result = m_shaderManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialized shader manager");

		result = m_modelManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialize model manager");

		result = m_meshManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialize mesh manager");

		result = m_pipelineManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialized pipeline manager");

		result = m_materialManager->Init(4096);
		TRC_ASSERT(result, "Failed to initialized material manager");



		return result;
	}
	void ResourceSystem::ShutDown()
	{
		m_meshManager->ShutDown();
		m_modelManager->ShutDown();
		m_materialManager->ShutDown();
		m_pipelineManager->ShutDown();
		m_shaderManager->ShutDown();
		m_fontManager->Shutdown();
		m_textureManager->ShutDown();

		SAFE_DELETE(m_materialManager, MaterialManager);
		SAFE_DELETE(m_pipelineManager, PipelineManager);
		SAFE_DELETE(m_meshManager, MeshManager);
		SAFE_DELETE(m_modelManager, ModelManager);
		SAFE_DELETE(m_shaderManager, ShaderManager);
		SAFE_DELETE(m_fontManager, FontManager);
		SAFE_DELETE(m_textureManager, TextureManager);

		
	}
	Texture_Ref ResourceSystem::GetDefaultTexture(const std::string& name)
	{
		return m_textureManager->GetDefault(name);
	}
	Texture_Ref ResourceSystem::CreateTexture(const std::string& name, TextureDesc desc)
	{
		if (m_textureManager->CreateTexture(name, desc))
		{
			return { m_textureManager->GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
		}
		TRC_ERROR("Failed to create Texture {}", name);
		return { nullptr, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
	}
	Texture_Ref ResourceSystem::LoadTexture(const std::string& name, TextureDesc desc)
	{

		if (m_textureManager->LoadTexture(name, desc))
		{
			return { m_textureManager->GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
		}
		
		TRC_WARN("Failed to load texture {}", name);

		return { nullptr, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
	}
	Texture_Ref ResourceSystem::LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc, const std::string& name)
	{
		if (m_textureManager->LoadTexture(filenames,desc,name))
		{
			return { m_textureManager->GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
		}

		TRC_WARN("Failed to load texture {}", name.c_str());

		return { nullptr, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
	}
	Texture_Ref ResourceSystem::LoadTexture(const std::string& name)
	{
		if (m_textureManager->LoadTexture(name))
		{
			return { m_textureManager->GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
		}

		TRC_WARN("Failed to load texture {}", name.c_str());

		return { nullptr, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager) };
	}
	Texture_Ref ResourceSystem::GetTexture(const std::string& name)
	{
		return { m_textureManager->GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, m_textureManager)};
	}
	void ResourceSystem::ReleaseTexture(const std::string& name)
	{
		m_textureManager->ReleaseTexture(name);
	}
	Ref<Mesh> ResourceSystem::LoadMesh(const std::string& name)
	{
		if (m_meshManager->LoadMesh(name))
		{
			return { m_meshManager->GetMesh(name), BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, m_meshManager) };
		}

		TRC_ERROR("Failed to load mesh {}", name.c_str());

		return Ref<Mesh>();
	}
	Ref<Mesh> ResourceSystem::GetDefaultMesh(const std::string& name)
	{
		return m_meshManager->GetDefault(name);
	}
	Ref<Mesh> ResourceSystem::GetMesh(const std::string& name)
	{
		return { m_meshManager->GetMesh(name), BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, m_meshManager) };
	}
	bool ResourceSystem::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{
		return m_pipelineManager->CreatePipeline(desc, name, auto_fill);
	}
	Ref<GPipeline> ResourceSystem::GetPipeline(const std::string& name)
	{
		return { m_pipelineManager->GetPipeline(name), BIND_RESOURCE_UNLOAD_FN(PipelineManager::Unload, m_pipelineManager)};
	}
	Ref<GPipeline> ResourceSystem::GetDefaultPipeline(const std::string& name)
	{
		return m_pipelineManager->GetDefault(name);
	}
	bool ResourceSystem::CreateMaterial(const std::string& name, Material material, Ref<GPipeline> pipeline)
	{
		return m_materialManager->CreateMaterial(name, material, pipeline);
	}
	Ref<MaterialInstance> ResourceSystem::GetMaterial(const std::string& name)
	{
		return { m_materialManager->GetMaterial(name), BIND_RESOURCE_UNLOAD_FN(MaterialManager::Unload, m_materialManager)};
	}
	ResourceSystem* ResourceSystem::get_instance()
	{

		if (s_instance == nullptr)
		{
			s_instance = new ResourceSystem();
		}
		return s_instance;
	}
	bool ResourceSystem::LoadDefaults()
	{
		bool result = m_textureManager->LoadDefaultTextures();
		TRC_ASSERT(result, "Failed to load default textures");

		result = m_fontManager->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default fonts");

		result = m_meshManager->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default meshes");

		result = m_pipelineManager->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default pipelines");

		result = m_materialManager->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default materials");

		return result;
	}
	Ref<GShader> ResourceSystem::CreateShader(const std::string& name, ShaderStage shader_stage)
	{
		m_shaderManager->CreateShader(name, shader_stage);
		return { m_shaderManager->GetShader(name), BIND_RESOURCE_UNLOAD_FN(ShaderManager::UnloadShader, m_shaderManager)};
	}
	Ref<GShader> ResourceSystem::GetShader(const std::string& name)
	{
		return { m_shaderManager->GetShader(name), BIND_RESOURCE_UNLOAD_FN(ShaderManager::UnloadShader, m_shaderManager) };
	}
	std::string ResourceSystem::GetShaderResourcePath()
	{
		return m_shaderManager->GetShaderResourcePath();
	}
	Ref<Font> ResourceSystem::LoadFont(const std::string& name)
	{
		if (m_fontManager->LoadFont(name))
		{
			return { m_fontManager->GetFont(name), BIND_RESOURCE_UNLOAD_FN(FontManager::UnloadFont, m_fontManager) };
		}
		TRC_ERROR("Failed to load font, {}", name);
		return { nullptr , BIND_RESOURCE_UNLOAD_FN(FontManager::UnloadFont, m_fontManager) };
	}
	Ref<Font> ResourceSystem::GetFont(const std::string& name)
	{
		return { m_fontManager->GetFont(name), BIND_RESOURCE_UNLOAD_FN(FontManager::UnloadFont, m_fontManager) };
	}
	std::string ResourceSystem::GetFontResourcePath()
	{
		return m_fontManager->GetFontResourcePath();
	}
}