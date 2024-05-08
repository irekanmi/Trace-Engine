#include "pch.h"

#include "ResourceSystem.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "MeshManager.h"
#include "PipelineManager.h"
#include "MaterialManager.h"
#include "ShaderManager.h"
#include "FontManager.h"
#include "scene/SceneManager.h"
#include "AnimationsManager.h"
#include "PrefabManager.h"

namespace trace {

	
	bool ResourceSystem::Init()
	{
		bool result = true;

		//NOTE: The resource initializes the scene manager because scenes are also resources
		result = result && SceneManager::get_instance()->Init(256);

		//TODO: Configurable
		result = result && TextureManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialize texture manager");

		result = result && FontManager::get_instance()->Init(256);
		TRC_ASSERT(result, "Failed to initialized shader manager");

		result = result && ShaderManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialized shader manager");

		result = result && ModelManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialize model manager");

		result = result && MeshManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialize mesh manager");

		result = result && PipelineManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialized pipeline manager");

		result = result && MaterialManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialized material manager");

		result = result && AnimationsManager::get_instance()->Init(4096, 4096);
		TRC_ASSERT(result, "Failed to initialized animations manager");

		result = result && PrefabManager::get_instance()->Init(4096);
		TRC_ASSERT(result, "Failed to initialized Prefab manager");


		return result;
	}
	void ResourceSystem::ShutDown()
	{
		//NOTE: The resource destroys the scene manager because scenes are also resources

		PrefabManager::get_instance()->Shutdown();
		AnimationsManager::get_instance()->Shutdown();
		MeshManager::get_instance()->ShutDown();
		ModelManager::get_instance()->ShutDown();
		MaterialManager::get_instance()->ShutDown();
		PipelineManager::get_instance()->ShutDown();
		ShaderManager::get_instance()->ShutDown();
		FontManager::get_instance()->Shutdown();
		TextureManager::get_instance()->ShutDown();

		SceneManager* sceneManager = SceneManager::get_instance();
		sceneManager->Shutdown();
		SAFE_DELETE(sceneManager, SceneManager);
		SAFE_DELETE(PrefabManager::get_instance(), PrefabManager);
		SAFE_DELETE(AnimationsManager::get_instance(), AnimationsManager);
		SAFE_DELETE(MeshManager::get_instance(), MeshManager);
		SAFE_DELETE(ModelManager::get_instance(), ModelManager);
		SAFE_DELETE(MaterialManager::get_instance(), MaterialManager);
		SAFE_DELETE(PipelineManager::get_instance(), PipelineManager);
		SAFE_DELETE(ShaderManager::get_instance(), ShaderManager);
		SAFE_DELETE(FontManager::get_instance(), FontManager);
		SAFE_DELETE(TextureManager::get_instance(), TextureManager);

		
	}

	bool ResourceSystem::LoadDefaults()
	{
		bool result = true;
		result = result && TextureManager::get_instance()->LoadDefaultTextures();
		TRC_ASSERT(result, "Failed to load default textures");

		result = result && FontManager::get_instance()->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default fonts");

		result = result && PipelineManager::get_instance()->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default pipelines");

		result = result && MaterialManager::get_instance()->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default materials");

		result = result && MeshManager::get_instance()->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default meshes");

		result = result && AnimationsManager::get_instance()->LoadDefaults();
		TRC_ASSERT(result, "Failed to load default animations");

		

		

		return result;
	}

	bool ResourceSystem::LoadDefaults_Runtime()
	{
		bool result = true;

		result = result && PipelineManager::get_instance()->LoadDefaults_Runtime();
		TRC_ASSERT(result, "Failed to load default pipelines");

		return result;
	}
	
}