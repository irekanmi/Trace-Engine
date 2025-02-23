#pragma once

#include "yaml_util.h"
#include "reflection/TypeHash.h"
#include "reflection/Type.h"
#include "core/io/Logging.h"
#include "reflection/Member.h"

#include <cinttypes>
#include <type_traits>

namespace trace {

	namespace Reflection {
		template<typename T>
		void Serialize(T& object, void* location, void* member_info, uint16_t format);

		template<typename T>
		void Deserialize(T& object, void* location, void* member_info, uint16_t format);

	}

	// Helper function template to test `std::ostream <<`
	template <typename T>
	auto test_emit_operator(YAML::Emitter& emit, const T& obj) -> decltype(emit << obj, void(), std::true_type{});

	std::false_type test_emit_operator(...); // Fallback for invalid types

	// Check if a type has `std::ostream <<`
	template <typename T>
	constexpr bool has_emit_operator_v = decltype(test_emit_operator(std::declval<YAML::Emitter&>(), std::declval<const T&>()))::value;

	
	// Helper function template to test `std::ostream <<`
	template<typename T, typename Enable = void>
	struct has_yaml_as : public std::false_type
	{};

	template<typename T>
	struct has_yaml_as < T, std::void_t<decltype( YAML::convert<T>::decode( std::declval<YAML::Node>(), std::declval<T&>()))>> : public std::true_type
	{};

	template<typename T>
	constexpr bool has_yaml_as_v = has_yaml_as<T>::value;

	template<typename T>
	constexpr bool can_serialize = std::is_same_v<T, char> || std::is_enum_v<T> || has_emit_operator_v<T>;

	class YAMLTypeSerializer
	{

	public:

		template<typename T>
		static void SerializeTypeData(T& obj, void* location, void* member_info)
		{
			
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			std::string key = std::string(Reflection::TypeName<T>());

			

			if (member_info && (uintptr_t)member_info != (uintptr_t)0x00000000000000FF)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				key = mem_info.name;
			}
			
			if constexpr (std::is_same_v<T, char>)
			{
				if (!member_info)
				{
					emit << YAML::BeginMap;
				}

				if (member_info && (uintptr_t)member_info != (uintptr_t)0x00000000000000FF)
				{
					Reflection::Member& member = *(Reflection::Member*)member_info;
					std::string val = std::string(&obj, member.variable.GetArraySize());
					YAML::Binary dat((const unsigned char*)&obj, member.variable.GetArraySize());
					emit << YAML::Key << key << YAML::Value << dat;

				}
				else
				{
					emit << YAML::Key << key << YAML::Value << std::string(&obj);
				}

				if (!member_info)
				{
					emit << YAML::EndMap;
				}
			}
			else if constexpr (std::is_enum_v<T>)
			{
				if (!member_info)
				{
					emit << YAML::BeginMap;
				}

				emit << YAML::Key << key << YAML::Value << (int)obj;

				if (!member_info)
				{
					emit << YAML::EndMap;
				}
			}
			else if constexpr (Reflection::IsTypeContainer<T>{} || Reflection::IsTypeKeyValueContainer<T>{})
			{
				return;
			}
			else if constexpr (has_emit_operator_v<T>)
			{
				if (!member_info)
				{
					emit << YAML::BeginMap;
				}

				emit << YAML::Key << key << YAML::Value << obj;

				if (!member_info)
				{
					emit << YAML::EndMap;
				}
			}
			

			

		}

		static void SerializeContainerSize(uint32_t size, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;
			emit << size;
		}

		static void SerializeTypeID(uint64_t type_id, void* location, void* member_info, int32_t index = -1)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			if (!member_info)
			{
				emit << type_id;
			}
			else
			{
				emit << YAML::Key << "Type ID" << YAML::Value << type_id;
			}
		}

		static void SerializeNullMember(void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;
			Reflection::Member& mem_info = *(Reflection::Member*)member_info;

			emit << YAML::Key << mem_info.name << YAML::Value << float(0.0f);
		}

		template<typename T>
		static void SerializeContainerMember(T& obj, uint32_t index, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			if (!can_serialize<T> || Reflection::IsTypeContainer<T>{} || Reflection::IsTypeKeyValueContainer<T>{})
			{
				emit << YAML::BeginMap;
			}
			type_info.serializer(&obj, location, nullptr, (uint16_t)Reflection::SerializationFormat::YAML);
			if (!can_serialize<T> || Reflection::IsTypeContainer<T>{} || Reflection::IsTypeKeyValueContainer<T>{})
			{
				emit << YAML::EndMap;
			}
		}

		static void BeginContainer(std::string_view type_name, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			if (member_info)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				emit << YAML::Key << mem_info.name << YAML::Value << YAML::BeginSeq;
			}
			else
			{
				emit << YAML::Key << std::string(type_name) << YAML::Value << YAML::BeginSeq;
			}

		}
		static void EndContainer(void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			emit << YAML::EndSeq;
		}

		static void BeginKeyValueContainer(std::string_view type_name, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			if (member_info)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				emit << YAML::Key << mem_info.name << YAML::Value << YAML::BeginSeq;
			}
			else
			{
				emit << YAML::Key << std::string(type_name) << YAML::Value << YAML::BeginSeq;
			}
		}
		static void EndKeyValueContainer(void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			emit << YAML::EndSeq;
		}

		static void BeginTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;
			
			std::string key = std::string(type_name);
			if (member_info)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				key = mem_info.name;
			}

			emit << YAML::Key << key << YAML::Value << YAML::BeginMap;
		}

		static void EndTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			emit << YAML::EndMap;

		}

		static void BeginParent(void* location)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			
			emit << YAML::Key << "Parent" << YAML::Value << YAML::BeginMap;

		}

		static void EndParent(void* location)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;

			emit << YAML::EndMap;
		}

		static void GetParentLocation(void* location, char* out_location, void* member_info)
		{
			YAML::Node& node = *(YAML::Node*)location;
			YAML::Node member_node = node["Parent"];

			memcpy(out_location, &member_node, sizeof(YAML::Node));
		}


		template<typename Key, typename Value>
		static void SerializeKeyValuePair(Key& key, Value& value, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			YAML::Emitter& emit = *(YAML::Emitter*)location;
			
			emit << YAML::BeginMap;
			
			emit << YAML::Key << "Key" << YAML::Value;
			Reflection::Serialize(key, location, nullptr, Reflection::SerializationFormat::YAML);
			emit << YAML::Key << "Value" << YAML::Value;
			if (!can_serialize<Value> || Reflection::IsTypeContainer<Value>{} || Reflection::IsTypeKeyValueContainer<Value>{})
			{
				emit << YAML::BeginMap;
			}
			type_info.serializer(&value, location, nullptr, (uint16_t)Reflection::SerializationFormat::YAML);
			if (!can_serialize<Value> || Reflection::IsTypeContainer<Value>{} || Reflection::IsTypeKeyValueContainer<Value>{})
			{
				emit << YAML::EndMap;
			}
			

			emit << YAML::EndMap;
		}

		static void GetMemberLocation(void* location, char* out_location, void* member_info)
		{
			TRC_ASSERT(member_info, "Invalid Member Info, Funtion: {}", __FUNCTION__);
			YAML::Node& node = *(YAML::Node*)location;
			Reflection::Member& mem_info = *(Reflection::Member*)member_info;
			YAML::Node member_node = node[mem_info.name];

			memcpy(out_location, &member_node, sizeof(YAML::Node));

		}
		static void GetTypeMemberLocation(uint64_t type_id, std::string_view type_name, void* location, char* out_location, void* member_info)
		{
			YAML::Node& node = *(YAML::Node*)location;

			std::string key = std::string(type_name);

			if (member_info)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				key = mem_info.name;
			}

			YAML::Node type_node = node[key];

			memcpy(out_location, &type_node, sizeof(YAML::Node));
		}

		static bool CheckValidMemberPointer(void* location, void* member_info)
		{
			TRC_ASSERT(member_info, "Invalid Member Info, Funtion: {}", __FUNCTION__);
			YAML::Node& node = *(YAML::Node*)location;
			Reflection::Member& mem_info = *(Reflection::Member*)member_info;

			if (node.IsScalar())
			{
				return false;
			}
			return true;
		}

		template<typename T>
		static void DeserializeTypeData(T& obj, void* location, void* member_info)
		{
			YAML::Node& node = *(YAML::Node*)location;
			
			std::string key = std::string(Reflection::TypeName<T>());

			if (member_info)
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				key = mem_info.name;
			}

			if constexpr (std::is_same_v<T, char>)
			{
				if (member_info)
				{
					Reflection::Member& mem_info = *(Reflection::Member*)member_info;
					YAML::Binary str = node.as<YAML::Binary>();
					
					memcpy(&obj, str.data(), mem_info.variable.GetArraySize());
				}
				else
				{
					std::string str = node[key].as<std::string>();
					memcpy(&obj, str.data(), str.size());
				}
			}
			else if constexpr (std::is_enum_v<T>)
			{				
				obj = member_info ? (T)node.as<int>() : (T)node[key].as<int>();
			}
			else if constexpr (has_yaml_as_v<T>)
			{
				obj = member_info ? node.as<T>() : node[key].as<T>();
			}
			
		}

		static void DeserializeContainerSize(uint32_t& out_size, void* location, void* member_info)
		{
			YAML::Node& node = *(YAML::Node*)location;

			out_size = node[0].as<uint32_t>();
		}

		static void DeserializeTypeID(uint64_t& out_type_id, void* location, void* member_info, int32_t index = -1)
		{
			YAML::Node& node = *(YAML::Node*)location;
			
			if (index != -1)
			{
				out_type_id = node[index].as<uint64_t>();
			}
			else
			{
				Reflection::Member& mem_info = *(Reflection::Member*)member_info;
				out_type_id = node["Type ID"].as<uint64_t>();
			}
		}

		template<typename T>
		static void DeserializeContainerMember(T& obj, uint32_t index, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			YAML::Node& node = *(YAML::Node*)location;

			YAML::Node& member_node = node[index];
			//Reflection::Deserialize(obj, &member_node, nullptr, Reflection::SerializationFormat::YAML);
			type_info.deserializer(&obj, (void*)&member_node, nullptr, (uint16_t)Reflection::SerializationFormat::YAML);
		}

		template<typename Key, typename Value>
		static void DeserializeKeyValuePair(Key& key, Value& value, Reflection::TypeInfo& type_info, uint32_t index, void* location, void* member_info)
		{
			
			YAML::Node& node = *(YAML::Node*)location;
			YAML::Node map = node[index];
			YAML::Node key_node = map["Key"];
			Reflection::Deserialize(key, &key_node, nullptr, Reflection::SerializationFormat::YAML);
			YAML::Node value_node = map["Value"];
			//Reflection::Deserialize(value, &value_node, nullptr, Reflection::SerializationFormat::YAML);
			type_info.deserializer(&value, &value_node, nullptr, (uint16_t)Reflection::SerializationFormat::YAML);

		}

	private:
	protected:

	};

}
