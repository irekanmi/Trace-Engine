#include "pch.h"

#include "serialize/SceneSerializeFunctions.h"
#include "scripting/ScriptEngine.h"
#include "reflection/SerializeTypes.h"

namespace trace {

	template<typename... Component>
	void serialize_component_type_id(Entity entity, std::vector<UUID>& components_type_id)
	{

		([&]() {
			if (entity.HasComponent<Component>())
			{
				components_type_id.push_back(UUID(Reflection::TypeID<Component>()));
			}
			}(), ...);


	}

	template<typename... Component>
	void serialize_component_type_id(ComponentGroup<Component...>, Entity entity, std::vector<UUID>& components_type_id)
	{
		serialize_component_type_id<Component...>(entity, components_type_id);
	}

	template<typename... Component>
	void serialize_component_data(Entity entity, YAML::Emitter& emit)
	{

		([&]() {
			if (entity.HasComponent<Component>())
			{
				Component& comp = entity.GetComponent<Component>();
				Reflection::Serialize(comp, &emit, nullptr, Reflection::SerializationFormat::YAML);
			}
			}(), ...);


	}

	template<typename... Component>
	void serialize_component_data(ComponentGroup<Component...>, Entity entity, YAML::Emitter& emit)
	{
		serialize_component_data<Component...>(entity, emit);
	}

	template<typename... Component>
	void serialize_component_data(Entity entity, DataStream* stream)
	{

		([&]() {
			if (entity.HasComponent<Component>())
			{
				Component& comp = entity.GetComponent<Component>();
				Reflection::Serialize(comp, stream, nullptr, Reflection::SerializationFormat::BINARY);
			}
			}(), ...);


	}

	template<typename... Component>
	void serialize_component_data(ComponentGroup<Component...>, Entity entity, DataStream* stream)
	{
		serialize_component_data<Component...>(entity, stream);
	}

	void serialize_entity_components(Entity entity, YAML::Emitter& emit, Scene* scene)
	{
		emit << YAML::BeginMap;
		std::vector<UUID> components_type_id;

		serialize_component_type_id(AllComponents{}, entity, components_type_id);

		Reflection::Serialize(components_type_id, &emit, nullptr, Reflection::SerializationFormat::YAML);

		serialize_component_data(ComponentGroup<IDComponent, HierachyComponent>{}, entity, emit);
		serialize_component_data(AllComponents{}, entity, emit);

		serialize_entity_scripts(entity, emit, scene);

		emit << YAML::EndMap;
	}

	void serialize_entity_components_binary(Entity entity, DataStream* stream, Scene* scene)
	{
		std::vector<UUID> components_type_id;

		serialize_component_type_id(AllComponents{}, entity, components_type_id);

		Reflection::Serialize(components_type_id, stream, nullptr, Reflection::SerializationFormat::BINARY);

		serialize_component_data(ComponentGroup<IDComponent, HierachyComponent>{}, entity, stream);
		serialize_component_data(AllComponents{}, entity, stream);

		//serialize_entity_scripts_binary(entity, stream, scene);

	}

	void serialize_entity_scripts(Entity entity, YAML::Emitter& emit, Scene* scene)
	{
		//Scripts ----------------------------------------
		emit << YAML::Key << "Scripts" << YAML::Value;
		emit << YAML::BeginSeq;

		ScriptRegistry& script_registry = scene->GetScriptRegistry();

		script_registry.Iterate(entity.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
			{
				//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
				auto& fields_instances = script_registry.GetFieldInstances();
				auto& field_manager = fields_instances[script];
				auto field_it = field_manager.find(uuid);
				bool has_fields = !(field_it == field_manager.end());
				if (!has_fields)
				{
					TRC_WARN("entity id:{} does not have a field instance with script:{}", (uint64_t)uuid, script->GetScriptName());
				}
				emit << YAML::BeginMap;

				emit << YAML::Key << "Script Name" << YAML::Value << script->GetScriptName();
				emit << YAML::Key << "Script Values" << YAML::Value << YAML::BeginSeq; // Script values
				if (has_fields)
				{
					ScriptFieldInstance& ins = field_manager[uuid];
					for (auto& [name, field] : ins.GetFields())
					{
						emit << YAML::BeginMap;

						emit << YAML::Key << "Name" << YAML::Value << name;
						//emit << YAML::Key << "Type" << YAML::Value << (int)field.type;
						switch (field.type)
						{
						case ScriptFieldType::String:
						{
							break;
						}
						case ScriptFieldType::Bool:
						{
							bool data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Byte:
						{
							char data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Double:
						{
							double data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Char:
						{
							char data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Float:
						{
							float data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Int16:
						{
							int16_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Int32:
						{
							int32_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Int64:
						{
							int64_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::UInt16:
						{
							uint16_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::UInt32:
						{
							uint32_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::UInt64:
						{
							uint64_t data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Action:
						case ScriptFieldType::Prefab:
						{
							UUID data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Vec2:
						{
							glm::vec2 data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Vec3:
						{
							glm::vec3 data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}
						case ScriptFieldType::Vec4:
						{
							glm::vec4 data;
							ins.GetValue(name, data);
							emit << YAML::Key << "Value" << YAML::Value << data;
							break;
						}

						}

						emit << YAML::EndMap;
					}
				}
				emit << YAML::EndSeq; // Script values

				emit << YAML::EndMap;
			});

		emit << YAML::EndSeq;
		// --------------------------------------------------
	}

#define SERIALIZE_SCRIPTS   if (scene->IsRunning())					   \
							{										   \
								instance->GetFieldValue(name, data);   \
							}										   \
							else									   \
							{										   \
								ins->GetValue(name, data);			   \
							}										   \
							stream->Write(data);				       \

	void serialize_entity_scripts_binary(Entity entity, DataStream* stream, Scene* scene)
	{
		//Scripts ----------------------------------------
		ScriptRegistry& script_registry = scene->GetScriptRegistry();

		uint32_t count = 0;
		script_registry.Iterate(entity.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
			{
				count++;
			}
		);
		stream->Write<uint32_t>(count);

		script_registry.Iterate(entity.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
			{
				//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
				auto& fields_instances = script_registry.GetFieldInstances();
				auto& field_manager = fields_instances[script];
				auto field_it = field_manager.find(uuid);

				uint32_t script_str_count = static_cast<uint32_t>(script->GetScriptName().size());
				stream->Write<uint32_t>(script_str_count);
				stream->Write((void*)script->GetScriptName().data(), script_str_count);

				bool has_fields = !scene->IsRunning() ? !(field_it == field_manager.end()) : true;
				uint32_t field_count = 0;
				if (!has_fields)
				{
					TRC_WARN("entity id:{} does not have a field instance with script:{}", (uint64_t)uuid, script->GetScriptName());
					stream->Write<uint32_t>(field_count);
				}
				if (has_fields)
				{
					ScriptFieldInstance* ins = scene->IsRunning() ? nullptr : &field_manager[uuid];
					field_count = script->GetFields().size();
					stream->Write<uint32_t>(field_count);
					for (auto& [name, field] : script->GetFields())
					{
						uint32_t str_count = static_cast<uint32_t>(name.size());
						stream->Write<uint32_t>(str_count);
						stream->Write((void*)name.data(), str_count);
						switch (field.field_type)
						{
						case ScriptFieldType::String:
						{
							break;
						}
						case ScriptFieldType::Bool:
						{
							bool data;
							SERIALIZE_SCRIPTS;

							break;
						}
						case ScriptFieldType::Byte:
						{
							char data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Double:
						{
							double data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Char:
						{
							char data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Float:
						{
							float data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Int16:
						{
							int16_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Int32:
						{
							int32_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Int64:
						{
							int64_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::UInt16:
						{
							uint16_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::UInt32:
						{
							uint32_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::UInt64:
						{
							uint64_t data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Action:
						case ScriptFieldType::Prefab:
						{
							UUID data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Vec2:
						{
							glm::vec2 data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Vec3:
						{
							glm::vec3 data;
							SERIALIZE_SCRIPTS;
							break;
						}
						case ScriptFieldType::Vec4:
						{
							glm::vec4 data;
							SERIALIZE_SCRIPTS;
							break;
						}

						}

					}
				}

			});

		// --------------------------------------------------
	}

	static void SerializeEntityScripts_Binary(Entity entity, DataStream* stream)
	{
		serialize_entity_scripts_binary(entity, stream, entity.GetScene());

		for (auto& i : entity.GetComponent<HierachyComponent>().children)
		{
			Entity child = entity.GetScene()->GetEntity(i);
			SerializeEntityScripts_Binary(child, stream);
		}
	}

	static void SerializeEntity_Binary(Entity entity, DataStream* stream, uint32_t& entity_count)
	{

		serialize_entity_components_binary(entity, stream, entity.GetScene());

		for (auto& i : entity.GetComponent<HierachyComponent>().children)
		{
			Entity child = entity.GetScene()->GetEntity(i);
			SerializeEntity_Binary(child, stream, entity_count);
			entity_count++;
		}



	}

	void SerializeEntity(Entity entity, DataStream* stream)
	{
		if (!entity || !stream)
		{
			TRC_ERROR("Pass in valid parameters, Function: {}", __FUNCTION__);
			return;
		}

		uint32_t pos_1 = stream->GetPosition();
		uint32_t entity_count = 0;
		stream->Write<uint32_t>(entity_count);

		Scene* scene = entity.GetScene();
		Entity handle = entity;
		
		SerializeEntity_Binary(handle, stream, entity_count);
		SerializeEntityScripts_Binary(handle, stream);


		uint32_t pos_2 = stream->GetPosition();
		stream->SetPosition(pos_1);
		stream->Write<uint32_t>(entity_count);
		stream->SetPosition(pos_2);
	}

	static void DeserializeEntityScripts_Binary(Entity entity, DataStream* stream)
	{
		deserialize_entity_scripts_binary(entity, entity.GetScene(), stream);

		for (auto& i : entity.GetComponent<HierachyComponent>().children)
		{
			Entity child = entity.GetScene()->GetEntity(i);
			DeserializeEntityScripts_Binary(child, stream);
		}
	}

	Entity DeserializeEntity(Scene* scene, DataStream* stream)
	{
		if (!scene || !stream)
		{
			TRC_ERROR("Pass in valid parameters, Function: {}", __FUNCTION__);
			return Entity();
		}

		uint32_t entity_count = 0;
		stream->Read<uint32_t>(entity_count);
		Entity obj;

		std::unordered_map<UUID, UUID> entity_map;
		obj = deserialize_entity_components_binary(scene, stream);


		for (uint32_t i = 0; i < entity_count; i++)
		{
			deserialize_entity_components_binary(scene, stream);
		}

		DeserializeEntityScripts_Binary(obj, stream);

		return obj;
	}

}