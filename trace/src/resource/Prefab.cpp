#include "pch.h"

#include "Prefab.h"
#include "serialize/SceneSerializer.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "resource/GenericAssetManager.h"
#include "resource/PrefabManager.h"

#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace {

    bool Prefab::Create(Entity handle)
    {
        Prefab* asset = this;
        //Entity p = PrefabManager::get_instance()->GetScene()->DuplicateEntity(handle);
        std::unordered_map<UUID, UUID> prefab_map;
        Entity p = instancite_prefab_hierachy(PrefabManager::get_instance()->GetScene(), handle, Entity(), prefab_map);
        asset->SetHandle(p.GetID());
        return true;
    }

    void Prefab::Destroy()
    {
        Prefab* asset = this;
        Scene* prefab_scene = PrefabManager::get_instance()->GetScene();
        if (prefab_scene->GetEntity(asset->GetHandle()))
        {
            PrefabManager::get_instance()->GetScene()->DestroyEntity(prefab_scene->GetEntity(asset->GetHandle()));
        }
    }

    Ref<Prefab> Prefab::Deserialize(UUID id)
    {
        Ref<Prefab> result;
        if (AppSettings::is_editor)
        {
            std::string file_path = GetPathFromUUID(id).string();
            if (!file_path.empty())
            {
                result = SceneSerializer::DeserializePrefab(file_path);
            }
        }
        else
        {
            result = GenericAssetManager::get_instance()->Load_Runtime<Prefab>(id);
        }

        return result;
    }

    Ref<Prefab> Prefab::Deserialize(DataStream* stream)
    {
        return SceneSerializer::DeserializePrefab(stream);
    }

}
