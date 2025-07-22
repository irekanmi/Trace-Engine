#pragma once

#include "AssetManager.h"
#include "Prefab.h"
#include "scene/Entity.h"

namespace trace {

	class Scene;

	class PrefabManager
	{

	public:

		bool Init();
		void Shutdown();

		/*Ref<Prefab> Create(const std::string& name);
		Ref<Prefab> Create(const std::string& name, Entity handle);

		virtual Ref<Prefab> Get(const std::string& name) override;*/
		Scene* GetScene() { return m_prefabScene; }

		/*virtual void UnLoad(Resource* res) override;
		virtual Ref<Prefab> Load_Runtime(UUID id) override;
		void RenameAsset(Ref<Prefab> asset, const std::string& new_name);

		void SetAssetMap(std::unordered_map<UUID, AssetHeader>& map);*/

		static PrefabManager* get_instance();
	private:
		Scene* m_prefabScene;
		//std::unordered_map<UUID, AssetHeader> m_assetsMap;

	protected:

	};

}
