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

	template<typename... Component>
	void deserialize_component_data(Entity entity, YAML::detail::iterator_value& data, UUID& component_type_id)
	{

		([&]() {
			if (component_type_id == Reflection::TypeID<Component>())
			{
				Component& comp = entity.GetOrAddComponent<Component>();
				Reflection::Deserialize(comp, &data, nullptr, Reflection::SerializationFormat::YAML);
			}
			}(), ...);


	}

	template<typename... Component>
	void deserialize_component_data(ComponentGroup<Component...>, Entity entity, YAML::detail::iterator_value& data, UUID& component_type_id)
	{
		deserialize_component_data<Component...>(entity, data, component_type_id);
	}

	template<typename... Component>
	void deserialize_component_data(Entity entity, DataStream* stream, UUID& component_type_id)
	{

		([&]() {
			if (component_type_id == Reflection::TypeID<Component>())
			{
				Component& comp = entity.GetOrAddComponent<Component>();
				Reflection::Deserialize(comp, stream, nullptr, Reflection::SerializationFormat::BINARY);
			}
			}(), ...);


	}

	template<typename... Component>
	void deserialize_component_data(ComponentGroup<Component...>, Entity entity, DataStream* stream, UUID& component_type_id)
	{
		deserialize_component_data<Component...>(entity, stream, component_type_id);
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

		serialize_entity_scripts_binary(entity, stream, scene);

	}

	Entity deserialize_entity_components(Scene* scene, YAML::detail::iterator_value& entity)
	{
		std::vector<UUID> components_type_id;
		Reflection::Deserialize(components_type_id, &entity, nullptr, Reflection::SerializationFormat::YAML);

		IDComponent id;
		Reflection::Deserialize(id, &entity, nullptr, Reflection::SerializationFormat::YAML);
		HierachyComponent hi;
		Reflection::Deserialize(hi, &entity, nullptr, Reflection::SerializationFormat::YAML);
		Entity obj = scene->CreateEntity_UUID(id._id, "", hi.parent);
		obj.AddOrReplaceComponent<IDComponent>(id);
		obj.AddOrReplaceComponent<HierachyComponent>(hi);
		for (uint32_t i = 0; i < components_type_id.size(); i++)
		{
			deserialize_component_data(AllComponents{}, obj, entity, components_type_id[i]);
		}

		deserialize_entity_scripts(obj, scene, entity);

		return obj;
	}

	Entity deserialize_entity_components_binary(Scene* scene, DataStream* stream)
	{
		std::vector<UUID> components_type_id;
		Reflection::Deserialize(components_type_id, stream, nullptr, Reflection::SerializationFormat::BINARY);

		IDComponent id;
		Reflection::Deserialize(id, stream, nullptr, Reflection::SerializationFormat::BINARY);
		HierachyComponent hi;
		Reflection::Deserialize(hi, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Entity obj = scene->CreateEntity_UUID(id._id, "", hi.parent);
		obj.AddOrReplaceComponent<IDComponent>(id);
		obj.AddOrReplaceComponent<HierachyComponent>(hi);
		for (uint32_t i = 0; i < components_type_id.size(); i++)
		{
			deserialize_component_data(AllComponents{}, obj, stream, components_type_id[i]);
		}

		deserialize_entity_scripts_binary(obj, scene, stream);

		return obj;
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

				uint32_t script_str_count = static_cast<uint32_t>(script->GetScriptName().size() + 1);
				stream->Write<uint32_t>(script_str_count);
				stream->Write((void*)script->GetScriptName().data(), script_str_count);

				bool has_fields = !(field_it == field_manager.end());
				uint32_t field_count = 0;
				if (!has_fields)
				{
					TRC_WARN("entity id:{} does not have a field instance with script:{}", (uint64_t)uuid, script->GetScriptName());
					stream->Write<uint32_t>(field_count);
				}
				if (has_fields)
				{
					ScriptFieldInstance& ins = field_manager[uuid];
					field_count = static_cast<uint32_t>(ins.GetFields().size());
					stream->Write<uint32_t>(field_count);
					for (auto& [name, field] : ins.GetFields())
					{
						uint32_t str_count = static_cast<uint32_t>(name.size() + 1);
						stream->Write<uint32_t>(str_count);
						stream->Write((void*)name.data(), str_count);
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
							stream->Write<bool>(data);
							break;
						}
						case ScriptFieldType::Byte:
						{
							char data;
							ins.GetValue(name, data);
							stream->Write<char>(data);
							break;
						}
						case ScriptFieldType::Double:
						{
							double data;
							ins.GetValue(name, data);
							stream->Write<double>(data);
							break;
						}
						case ScriptFieldType::Char:
						{
							char data;
							ins.GetValue(name, data);
							stream->Write<char>(data);
							break;
						}
						case ScriptFieldType::Float:
						{
							float data;
							ins.GetValue(name, data);
							stream->Write<float>(data);
							break;
						}
						case ScriptFieldType::Int16:
						{
							int16_t data;
							ins.GetValue(name, data);
							stream->Write<int16_t>(data);
							break;
						}
						case ScriptFieldType::Int32:
						{
							int32_t data;
							ins.GetValue(name, data);
							stream->Write<int32_t>(data);
							break;
						}
						case ScriptFieldType::Int64:
						{
							int64_t data;
							ins.GetValue(name, data);
							stream->Write<int64_t>(data);
							break;
						}
						case ScriptFieldType::UInt16:
						{
							uint16_t data;
							ins.GetValue(name, data);
							stream->Write<uint16_t>(data);
							break;
						}
						case ScriptFieldType::UInt32:
						{
							uint32_t data;
							ins.GetValue(name, data);
							stream->Write<uint32_t>(data);
							break;
						}
						case ScriptFieldType::UInt64:
						{
							uint64_t data;
							ins.GetValue(name, data);
							stream->Write<uint64_t>(data);
							break;
						}
						case ScriptFieldType::Vec2:
						{
							glm::vec2 data;
							ins.GetValue(name, data);
							stream->Write<glm::vec2>(data);
							break;
						}
						case ScriptFieldType::Vec3:
						{
							glm::vec3 data;
							ins.GetValue(name, data);
							stream->Write<glm::vec3>(data);
							break;
						}
						case ScriptFieldType::Vec4:
						{
							glm::vec4 data;
							ins.GetValue(name, data);
							stream->Write<glm::vec4>(data);
							break;
						}

						}

					}
				}

			});

		// --------------------------------------------------
	}

	void deserialize_entity_scripts(Entity obj, Scene* scene, YAML::detail::iterator_value& entity)
	{
		UUID uuid = obj.GetID();
		YAML::Node scripts = entity["Scripts"];
		if (scripts)
		{
			std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();

			for (auto script : scripts)
			{
				std::string script_name = script["Script Name"].as<std::string>();
				auto it = g_Scripts.find(script_name);
				if (it == g_Scripts.end())
				{
					TRC_WARN("Script:{} does not exist in the assembly", script_name);
					continue;
				}
				obj.AddScript(script_name);

				ScriptRegistry& script_registry = scene->GetScriptRegistry();

				//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
				auto& fields_instances = script_registry.GetFieldInstances();
				auto& field_manager = fields_instances[&it->second];
				auto field_it = field_manager.find(uuid);
				if (field_it == field_manager.end())
				{
					ScriptFieldInstance& field_ins = field_manager[obj.GetID()];
					field_ins.Init(&it->second);
				}
				ScriptFieldInstance& ins = field_manager[obj.GetID()];

				for (auto values : script["Script Values"])
				{
					std::string field_name = values["Name"].as<std::string>();
					auto f_it = ins.GetFields().find(field_name);
					if (f_it == ins.GetFields().end())
					{
						TRC_WARN("Script Field:{} does not exist in the Script Class", field_name);
						continue;
					}
					switch (f_it->second.type)
					{
					case ScriptFieldType::String:
					{
						break;
					}
					case ScriptFieldType::Bool:
					{
						bool data = values["Value"].as<bool>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Byte:
					{
						char data = values["Value"].as<char>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Double:
					{
						double data = values["Value"].as<double>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Char:
					{
						char data = values["Value"].as<char>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Float:
					{
						float data = values["Value"].as<float>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int16:
					{
						int16_t data = values["Value"].as<int16_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int32:
					{
						int32_t data = values["Value"].as<int32_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int64:
					{
						int64_t data = values["Value"].as<int64_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt16:
					{
						uint16_t data = values["Value"].as<uint16_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt32:
					{
						uint32_t data = values["Value"].as<uint32_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt64:
					{
						uint64_t data = values["Value"].as<uint64_t>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec2:
					{
						glm::vec2 data = values["Value"].as<glm::vec2>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec3:
					{
						glm::vec3 data = values["Value"].as<glm::vec3>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec4:
					{
						glm::vec4 data = values["Value"].as<glm::vec4>();
						ins.SetValue(field_name, data);
						break;
					}
					}
				}


			}
		}
	}

	void deserialize_entity_scripts_binary(Entity obj, Scene* scene, DataStream* stream)
	{
		UUID uuid = obj.GetID();
		uint32_t script_count = 0;
		stream->Read<uint32_t>(script_count);
		if (script_count > 0)
		{
			std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();

			for (uint32_t i = 0; i < script_count; i++)
			{
				uint32_t script_str_count = 0;
				std::string script_name;
				stream->Read<uint32_t>(script_str_count);
				script_name.reserve(script_str_count);
				script_name.resize(script_str_count - 1);
				stream->Read((void*)script_name.data(), script_str_count);
				auto it = g_Scripts.find(script_name);
				if (it == g_Scripts.end())
				{
					TRC_ASSERT(false, "Script:{} does not exist in the assembly", script_name);
					continue;
				}
				obj.AddScript(script_name);

				ScriptRegistry& script_registry = scene->GetScriptRegistry();

				//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
				auto& fields_instances = script_registry.GetFieldInstances();
				auto& field_manager = fields_instances[&it->second];
				auto field_it = field_manager.find(uuid);
				if (field_it == field_manager.end())
				{
					ScriptFieldInstance& field_ins = field_manager[obj.GetID()];
					field_ins.Init(&it->second);
				}
				ScriptFieldInstance& ins = field_manager[obj.GetID()];
				uint32_t field_count = 0;
				stream->Read<uint32_t>(field_count);
				for (uint32_t i = 0; i < field_count; i++)
				{
					uint32_t field_str_count = 0;
					std::string field_name;
					stream->Read<uint32_t>(field_str_count);
					field_name.reserve(field_str_count);
					field_name.resize(field_str_count - 1);
					stream->Read((void*)field_name.data(), field_str_count);
					auto f_it = ins.GetFields().find(field_name);
					if (f_it == ins.GetFields().end())
					{
						TRC_ASSERT(false, "Script Field:{} does not exist in the Script Class", field_name);
						continue;
					}
					switch (f_it->second.type)
					{
					case ScriptFieldType::String:
					{
						break;
					}
					case ScriptFieldType::Bool:
					{
						bool data;
						stream->Read<bool>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Byte:
					{
						char data;
						stream->Read<char>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Double:
					{
						double data;
						stream->Read<double>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Char:
					{
						char data;
						stream->Read<char>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Float:
					{
						float data;
						stream->Read<float>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int16:
					{
						int16_t data;
						stream->Read<int16_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int32:
					{
						int32_t data;
						stream->Read<int32_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Int64:
					{
						int64_t data;
						stream->Read<int64_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt16:
					{
						uint16_t data;
						stream->Read<uint16_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt32:
					{
						uint32_t data;
						stream->Read<uint32_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::UInt64:
					{
						uint64_t data;
						stream->Read<uint64_t>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec2:
					{
						glm::vec2 data;
						stream->Read<glm::vec2>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec3:
					{
						glm::vec3 data;
						stream->Read<glm::vec3>(data);
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Vec4:
					{
						glm::vec4 data;
						stream->Read<glm::vec4>(data);
						ins.SetValue(field_name, data);
						break;
					}
					}
				}


			}
		}
	}
}