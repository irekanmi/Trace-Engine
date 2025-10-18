#include "pch.h"

#include "serialize/SceneSerializeFunctions.h"
#include "scripting/ScriptEngine.h"
#include "reflection/SerializeTypes.h"

namespace trace {

	
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

		//deserialize_entity_scripts_binary(obj, scene, stream);

		return obj;
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
					case ScriptFieldType::Action:
					{
						UUID data = values["Value"].as<UUID>();
						ins.SetValue(field_name, data);
						break;
					}
					case ScriptFieldType::Prefab:
					{
						UUID data = values["Value"].as<UUID>();
						ins.SetValue(field_name, data);
						if (data != 0)
						{
							Ref<Prefab> asset = Prefab::Deserialize(data);
							if (asset)
							{
								asset->Increment();
							}
						}
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

#define DESERIALIZE_SCRIPT stream->Read(data);						             \
	                       if (scene->IsRunning())                               \
							{                                                    \
								obj_ins->SetFieldValue(field_name, data);        \
							}                                                    \
							else                                                 \
							{													 \
								ins.SetValue(field_name, data);					 \
							}													 \

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
				script_name.resize(script_str_count);
				stream->Read((void*)script_name.data(), script_str_count);
				auto it = g_Scripts.find(script_name);
				if (it == g_Scripts.end())
				{
					TRC_ASSERT(false, "Script:{} does not exist in the assembly", script_name);
					continue;
				}
				ScriptInstance* obj_ins = obj.AddScript(script_name);

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
					field_name.resize(field_str_count);
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
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Byte:
					{
						char data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Double:
					{
						double data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Char:
					{
						char data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Float:
					{
						float data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Int16:
					{
						int16_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Int32:
					{
						int32_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Int64:
					{
						int64_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::UInt16:
					{
						uint16_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::UInt32:
					{
						uint32_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::UInt64:
					{
						uint64_t data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Action:
					{
						UUID data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Prefab:
					{
						UUID data;
						DESERIALIZE_SCRIPT;
						if (data != 0)
						{
							Ref<Prefab> asset = Prefab::Deserialize(data);
							if (asset)
							{
								asset->Increment();
							}
						}
						break;
					}
					case ScriptFieldType::Vec2:
					{
						glm::vec2 data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Vec3:
					{
						glm::vec3 data;
						DESERIALIZE_SCRIPT;
						break;
					}
					case ScriptFieldType::Vec4:
					{
						glm::vec4 data;
						DESERIALIZE_SCRIPT;
						break;
					}
					}
				}


			}
		}
	}


}