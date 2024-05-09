#pragma once

#include "AssetManager.h"
#include "Prefab.h"
#include "scene/Entity.h"

namespace trace {

	class Scene;

	class PrefabManager : AssetManager<Prefab>
	{

	public:

		virtual bool Init(uint32_t num_units) override;
		virtual void Shutdown() override;

		Ref<Prefab> Create(const std::string& name);
		Ref<Prefab> Create(const std::string& name, Entity handle);

		virtual Ref<Prefab> Get(const std::string& name) override;
		Scene* GetScene() { return m_prefabScene; }

		virtual void UnLoad(Prefab* asset) override;
		virtual Ref<Prefab> Load_Runtime(UUID id) override;

		static PrefabManager* get_instance();
	private:
		Scene* m_prefabScene;

	protected:

	};

}
