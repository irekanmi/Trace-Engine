#pragma once

#include "AssetManager.h"
#include "Prefab.h"

namespace trace {

	class Scene;

	class PrefabManager : AssetManager<Prefab>
	{

	public:

		virtual bool Init(uint32_t num_units) override;
		virtual void Shutdown() override;

		Ref<Prefab> Create(const std::string& name);

		virtual Ref<Prefab> Get(const std::string& name) override;

		virtual void UnLoad(Prefab* asset) override;
		virtual Ref<Prefab> Load_Runtime(UUID id) override;

		static PrefabManager* get_instance();
	private:
		Scene* m_prefabScene;

	protected:

	};

}
