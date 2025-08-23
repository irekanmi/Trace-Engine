#include "pch.h"

#include "ResourceSystem.h"
#include "PrefabManager.h"
#include "GenericAssetManager.h"
#include "DefaultAssetsManager.h"
#include "backends/Fontutils.h"

namespace trace {

	
	bool ResourceSystem::Init()
	{
		bool result = true;
		result = result && PrefabManager::get_instance()->Init();
		TRC_ASSERT(result, "Failed to initialized Prefab manager");

		result = result && GenericAssetManager::get_instance()->Init(8192);
		TRC_ASSERT(result, "Failed to initialized GenericAssetManager manager");

		FontFuncLoader::Load_MSDF_Func();

		if (AppSettings::is_editor)
		{
			DefaultAssetsManager::LoadAssetsPath();
		}

		return result;
	}
	void ResourceSystem::ShutDown()
	{
		DefaultAssetsManager::ReleaseAssets();

		PrefabManager::get_instance()->Shutdown();
		GenericAssetManager::get_instance()->Shutdown();
		
		SAFE_DELETE(GenericAssetManager::get_instance(), GenericAssetManager);
		SAFE_DELETE(PrefabManager::get_instance(), PrefabManager);
		

		
	}

	bool ResourceSystem::LoadDefaults()
	{
		bool result = true;
		DefaultAssetsManager::LoadAssets();

		

		return result;
	}

	bool ResourceSystem::LoadDefaults_Runtime()
	{
		bool result = true;
		DefaultAssetsManager::LoadAssets_Runtime();

		return result;
	}
	
}