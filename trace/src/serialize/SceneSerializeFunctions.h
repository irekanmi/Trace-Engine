#pragma once

#include "scene/Entity.h"
#include "scene/Scene.h"
#include "serialize/DataStream.h"
#include "yaml_util.h"

namespace trace {

	void serialize_entity_components(Entity entity, YAML::Emitter& emit, Scene* scene);
	void serialize_entity_components_binary(Entity entity, DataStream* stream, Scene* scene);
	Entity deserialize_entity_components(Scene* scene, YAML::detail::iterator_value& entity);
	Entity deserialize_entity_components_binary(Scene* scene, DataStream* stream);

	void serialize_entity_scripts(Entity entity, YAML::Emitter& emit, Scene* scene);
	void serialize_entity_scripts_binary(Entity entity, DataStream* stream, Scene* scene);
	void deserialize_entity_scripts(Entity obj, Scene* scene, YAML::detail::iterator_value& entity);
	void deserialize_entity_scripts_binary(Entity obj, Scene* scene, DataStream* stream);

	void SerializeEntity(Entity entity, DataStream* stream);
	Entity DeserializeEntity(Scene* scene, DataStream* stream);
}
