#include "pch.h"

#include "Prefab.h"
#include "serialize/SceneSerializer.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace {


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

        }

        return result;
    }

}
