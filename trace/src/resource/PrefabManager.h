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

		
		Scene* GetScene() { return m_prefabScene; }


		static PrefabManager* get_instance();
	private:
		Scene* m_prefabScene;

	protected:

	};

}
