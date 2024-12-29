#pragma once

#include "TypeHash.h"
#include "serialize/YAMLTypeSerializer.h"
#include "core/io/Logging.h"
#include "Serialize.h"
#include "resource/Ref.h"
#include "Type.h"
#include "core/Coretypes.h"
#include "core/Utils.h"

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <iostream>
#include <type_traits>

namespace trace {
	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);
}

namespace trace::Reflection {
	 

	template<typename = void, typename... Args>
	struct test_func : std::false_type {};

	template <typename F, typename... Args>
	struct test_func<std::void_t<decltype(std::declval<F>()(std::declval<Args>()...))>, F, Args...>
		: std::true_type {};

	template<typename... Args>
	inline constexpr bool test_func_v = test_func<void, Args...>::value;

	// Helper function template to test `GetTypeID`
	template<typename T, typename Enable = void>
	struct has_get_id : public std::false_type
	{};

	template<typename T>
	struct has_get_id < T, std::void_t<decltype(std::declval<T>().GetTypeID()) >> : public std::true_type
	{};

	template<typename T>
	constexpr bool has_get_id_v = has_get_id<T>::value;

	

	// SERIALIZATION ------------------------------------------------------------------

	template<typename T>
	static void SerializeTypeData(T& obj, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::SerializeTypeData(obj);
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::SerializeTypeData(obj, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeTypeData(obj, location, member_info);
			break;
		}

		}

	}

	static void SerializeContainerSize(uint32_t size, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::SerializeContainerSize(size);
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::SerializeContainerSize(size, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeContainerSize(size, location, member_info);
			break;
		}

		}
	}

	static void SerializeNullMember(void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeNullMember(location, member_info);
			break;
		}

		}
	}

	static void BeginContainer(std::string_view type_name, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::BeginContainer();
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::BeginContainer(location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::BeginContainer(type_name, location, member_info);
			break;
		}

		}
	}
	static void EndContainer(void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::EndContainer();
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::EndContainer(location);
			break;
		}

		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::EndContainer(location, member_info);
			break;
		}

		}
	}

	static void BeginKeyValueContainer(std::string_view type_name, void* location, void* member_info, uint16_t format)
	{

		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::BeginKeyValueContainer();
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::BeginKeyValueContainer(location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::BeginKeyValueContainer(type_name, location, member_info);
			break;
		}

		}

	}
	static void EndKeyValueContainer(void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::EndKeyValueContainer();
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::EndKeyValueContainer(location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::EndKeyValueContainer(location, member_info);
			break;
		}

		}
	}

	static void BeginTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::BeginTypeMembers(type_id, type_name);
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::BeginTypeMembers(type_id, type_name, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::BeginTypeMembers(type_id, type_name, location, member_info);
			break;
		}

		}
	}

	static void EndTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::EndTypeMembers(type_id, type_name);
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::EndTypeMembers(type_id, type_name, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::EndTypeMembers(type_id, type_name, location, member_info);
			break;
		}

		}
	}

	static void BeginParent(void* location, uint16_t format)
	{
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::BeginParent(location);
			break;
		}
		}
	}

	static void EndParent(void* location, uint16_t format)
	{
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::EndParent(location);
			break;
		}
		}
	}

	template<typename T>
	static void SerializeContainerMember(T& obj, uint32_t index, TypeInfo& type_info, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeContainerMember(obj, index, type_info, location, member_info);
			break;
		}

		}
	}

	template<typename Key, typename Value>
	static void SerializeKeyValuePair(Key& key, Value& value, TypeInfo& type_info, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case 0:
		{
			ConsoleSerializer::SerializeKeyValuePair(key, value);
			break;
		}
		case SerializationFormat::BINARY:
		{
			BinarySerializer::SerializeKeyValuePair(key, value, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeKeyValuePair(key, value, type_info, location, member_info);
			break;
		}

		}
	}

	static void SerializeTypeID(uint64_t type_id, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::SerializeTypeID(type_id, location, member_info);
			break;
		}

		}
	}

	// =================================================================================


	template<typename T>
	void SerializeContainer(T& object, void* location, void* member_info, uint16_t format)
	{
		TRC_ASSERT(false, "This type is not a container type");
	}

	template<typename T>
	void Serialize(T& object, void* location, void* member_info, uint16_t format)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			TypeInfo& type_info = TypeRegistry::GetTypesData().data[object->GetTypeID()];

			type_info.serializer(object, location, member_info, format);

			return;
		}

		constexpr uint64_t type_id = TypeID<T>();
		if (!TypeRegistry::HasType(type_id))
		{
			TRC_TRACE("Type has not registered -> Type Name: {}", TypeName<T>());
			RegisterTypeObject<T> type_registered;
		}

		TypeInfo& info = TypeRegistry::GetTypesData().data[type_id];

		

		if (info.IsContainer() || info.IsKeyValueContainer())
		{
			SerializeContainer(object, location, member_info, format);
			return;
		}

		if (TypeRegistry::HasMembers(type_id))
		{
			BeginTypeMembers(type_id, info.name, location, member_info, format);
			
			uint32_t index = 0;
			if (member_info)
			{
				Member& class_info = *(Member*)member_info;
				if (class_info.variable.IsPointer())
				{
					SerializeTypeID(type_id, location, member_info, format);
					index = 1;
				}
			}
			if (info.HasParent())
			{
				auto it = TypeRegistry::GetTypesData().data.find(info.parent_class);

				if (it != TypeRegistry::GetTypesData().data.end())
				{
					BeginParent(location, format);
					it->second.serializer(&object, location, nullptr, format);
					EndParent(location, format);
				}

			}
			for (auto& member : TypeRegistry::GetTypesData().members_data[type_id])
			{
				void* member_location = (char*)&object + member.offset;
				TypeInfo& member_type_info = TypeRegistry::GetTypesData().data[member.type_id];
				member_type_info.serializer(member_location, location, (void*)&member, format);

				index++;
			}
			EndTypeMembers(type_id, info.name, location, member_info, format);
		}
		else if (!TypeRegistry::HasMembers(type_id) && info.HasParent())
		{
			BeginTypeMembers(type_id, info.name, location, member_info, format);
			if (member_info)
			{
				Member& class_info = *(Member*)member_info;
				if (class_info.variable.IsPointer())
				{
					SerializeTypeID(type_id, location, member_info, format);
				}
			}
			if (info.HasParent())
			{
				auto it = TypeRegistry::GetTypesData().data.find(info.parent_class);

				if (it != TypeRegistry::GetTypesData().data.end())
				{
					BeginParent(location, format);
					it->second.serializer(&object, location, nullptr, format);
					EndParent(location, format);
				}

			}

			EndTypeMembers(type_id, info.name, location, member_info, format);
		}
		else
		{
			SerializeTypeData(object, location, member_info, format);
		}


	}

	template<typename T>
	void Serialize(T*& object, void* location, void* member_info, uint16_t format)
	{
		//TRC_ASSERT(object != nullptr, "Invalid object pointer");
		if (object != nullptr)
		{
			if constexpr (has_get_id_v<T>)
			{
				uint64_t type_id = TypeID<T>();
				uint64_t actual_type_id = object->GetTypeID();

				TypeInfo t_info = TypeRegistry::GetTypesData().data[actual_type_id];

				if (type_id == actual_type_id)
				{
					Serialize(*object, location, member_info, format);
				}
				else
				{
					t_info.serializer(&object, location, member_info, format);
				}
			}
			else
			{
				Serialize(*object, location, member_info, format);
			}
		}
		else if (member_info && object == nullptr)
		{
			SerializeNullMember(location, member_info, format);
		}

	}

	template<typename T>
	void Serialize(Ref<T>& object, void* location, void* member_info, uint16_t format)
	{
		if constexpr (std::is_base_of_v<Resource, T>)
		{
			UUID id = 0;
			if (object)
			{
				id = GetUUIDFromName(object->GetName());
			}

			//HACK: the is an hack to ensure proper serialization for YAML format
			void* info = member_info;
			if (!member_info)
			{
				uintptr_t hack = 0x00000000000000FF;
				info = (void*)hack;
			}
			Serialize(id, location, info, format);
		}

	}

	template<>
	void Serialize<StringID>(StringID& object, void* location, void* member_info, uint16_t format);

	template<typename T, typename Alloc>
	void SerializeContainer(std::vector<T, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		uint32_t size = object.size();
		constexpr std::string_view type_name = TypeName< std::vector<T, Alloc>>();
		BeginContainer(type_name, location, member_info, format);
		SerializeContainerSize(size, location, member_info, format);

		if (size > 0)
		{
			uint32_t index = 1;
			using type = remove_all_pointers_t<T>;
			uint64_t type_id = TypeID<type>();

			TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

			using value_type = std::remove_cv_t<T>;
			for (auto& member : object)
			{
				value_type value_data = member;
				if constexpr (has_get_id_v<type> && std::is_pointer_v<T>)
				{
					type_id = object[(index - 1)]->GetTypeID();
					SerializeTypeID(type_id, location, nullptr, format);
					t_info = TypeRegistry::GetTypesData().data[type_id];
					SerializeContainerMember(*member, index, t_info, location, nullptr, format);
				}
				else
				{
					SerializeContainerMember(member, index, t_info, location, nullptr, format);
				}

				index++;
			}
		}
		EndContainer(location, member_info, format);
	}

	template<typename T, typename Compare, typename Alloc>
	void SerializeContainer(std::set<T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		uint32_t size = object.size();
		constexpr std::string_view type_name = TypeName<std::set<T, Compare, Alloc>>();
		BeginContainer(type_name, location, member_info, format);
		SerializeContainerSize(size, location, member_info, format);

		if (size > 0)
		{
			uint32_t index = 1;
			using type = remove_all_pointers_t<T>;
			uint64_t type_id = TypeID<type>();

			TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

			using value_type = std::remove_cv_t<T>;
			for (auto& member : object)
			{
				value_type value_data = member;
				if constexpr (has_get_id_v<type> && std::is_pointer_v<T>)
				{
					type_id = object[(index - 1)]->GetTypeID();
					SerializeTypeID(type_id, location, nullptr, format);
					t_info = TypeRegistry::GetTypesData().data[type_id];
					SerializeContainerMember(*member, index, t_info, location, nullptr, format);
				}
				else
				{
					SerializeContainerMember(member, index, t_info, location, nullptr, format);
				}

				index++;
			}
		}
		EndContainer(location, member_info, format);
	}

	template<typename T, typename Compare, typename Alloc>
	void SerializeContainer(std::unordered_set<T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		uint32_t size = object.size();
		constexpr std::string_view type_name = TypeName<std::unordered_set<T, Compare, Alloc>>();
		BeginContainer(type_name, location, member_info, format);
		SerializeContainerSize(size, location, member_info, format);

		if (size > 0)
		{
			uint32_t index = 1;
			using type = remove_all_pointers_t<T>;
			uint64_t type_id = TypeID<type>();
			

			TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

			using value_type = std::remove_cv_t<T>;
			for (auto& member : object)
			{
				value_type value_data = member;
				if constexpr (has_get_id_v<type> && std::is_pointer_v<T>)
				{
					type_id = object[(index - 1)]->GetTypeID();
					SerializeTypeID(type_id, location, nullptr, format);
					t_info = TypeRegistry::GetTypesData().data[type_id];
					SerializeContainerMember(*member, index, t_info, location, nullptr, format);
				}
				else
				{
					SerializeContainerMember(member, index, t_info, location, nullptr, format);
				}

				index++;
			}
		}
		EndContainer(location, member_info, format);
	}

	template<typename Key, typename T, typename Compare, typename Alloc>
	void SerializeContainer(std::map<Key, T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		using key_type = std::remove_cv_t<Key>;
		uint32_t size = object.size();
		constexpr std::string_view type_name = TypeName< std::map<Key, T, Compare, Alloc>>();
		BeginKeyValueContainer(type_name,location,member_info, format);
		SerializeContainerSize(size, location, member_info, format);

		if (size > 0)
		{
			uint32_t index = 1;
			using type = remove_all_pointers_t<T>;
			uint64_t type_id = TypeID<type>();

			TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];
			for (auto& [key, value] : object)
			{
				key_type key_value = key;

				if constexpr (has_get_id_v<type> && std::is_pointer_v<T>)
				{
					type_id = value->GetTypeID();
					t_info = TypeRegistry::GetTypesData().data[type_id];
					SerializeTypeID(type_id, location, nullptr, format);
					SerializeKeyValuePair(key_value, *value, t_info, location, nullptr, format);
				}
				else
				{
					SerializeKeyValuePair(key_value, value, t_info, location, nullptr, format);
				}
			}
		}
		EndKeyValueContainer(location, member_info, format);
	}

	template<typename Key, typename T, typename Compare, typename Alloc>
	void SerializeContainer(std::unordered_map<Key, T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		using key_type = std::remove_cv_t<Key>;
		uint32_t size = object.size();
		constexpr std::string_view type_name = TypeName< std::unordered_map<Key, T, Compare, Alloc>>();
		BeginKeyValueContainer(type_name, location, member_info, format);
		SerializeContainerSize(size, location, member_info, format);

		if (size > 0)
		{
			uint32_t index = 1;
			using type = remove_all_pointers_t<T>;
			uint64_t type_id = TypeID<type>();

			TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];
			for (auto& [key, value] : object)
			{
				key_type key_value = key;

				if constexpr (has_get_id_v<type> && std::is_pointer_v<T>)
				{
					type_id = value->GetTypeID();
					t_info = TypeRegistry::GetTypesData().data[type_id];
					SerializeTypeID(type_id, location, nullptr, format);
					SerializeKeyValuePair(key_value, *value, t_info, location, nullptr, format);
				}
				else
				{
					SerializeKeyValuePair(key_value, value, t_info, location, nullptr, format);
				}
			}
		}
		EndKeyValueContainer(location, member_info, format);
	}

	

	// --------------------------------------------------------------------------------


	// DESERIALIZATION ------------------------------------------------------------------

	template<typename T>
	static void DeserializeTypeData(T& obj, void* location, void* member_info, uint16_t format)
	{

		switch (format)
		{

		case SerializationFormat::BINARY:
		{
			BinarySerializer::DeserializeTypeData(obj, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::DeserializeTypeData(obj, location, member_info);
			break;
		}

		}

	}

	static void DeserializeContainerSize(uint32_t& out_size, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case SerializationFormat::BINARY:
		{
			BinarySerializer::DeserializeContainerSize(out_size, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::DeserializeContainerSize(out_size, location, member_info);
			break;
		}

		}
	}

	template<typename T>
	static void DeserializeContainerMember(T& obj, uint32_t index, TypeInfo& type_info, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case SerializationFormat::BINARY:
		{
			BinarySerializer::DeserializeContainerMember(obj, index, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::DeserializeContainerMember(obj, index, type_info, location, member_info);
			break;
		}

		}
	}

	template<typename Key, typename Value>
	static void DeserializeKeyValuePair(Key& key, Value& value, TypeInfo& type_info, uint32_t index, void* location, void* member_info, uint16_t format)
	{
		switch (format)
		{

		case SerializationFormat::BINARY:
		{
			BinarySerializer::DeserializeKeyValuePair(key, value, index, location);
			break;
		}
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::DeserializeKeyValuePair(key, value, type_info, index, location, member_info);
			break;
		}

		}
	}

	static void GetMemberLocation(void* location, char* out_location, void* member_info, uint32_t format)
	{
		uintptr_t location_ptr = (uintptr_t)location;
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::GetMemberLocation(location, out_location, member_info);
			break;
		}
		default:
			memcpy(out_location, &location_ptr, sizeof(uintptr_t));
		}

	}

	static void GetTypeMemberLocation(uint64_t type_id, std::string_view type_name, void* location, char* out_location, void* member_info, uint32_t format)
	{
		uintptr_t location_ptr = (uintptr_t)location;
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::GetTypeMemberLocation(type_id, type_name, location, out_location, member_info);
			break;
		}
		default:
			memcpy(out_location, &location_ptr, sizeof(uintptr_t));
		}

	}

	static void DeserializeTypeID(uint64_t& out_type_id, void* location, void* member_info, uint16_t format, int32_t index = -1)
	{
		switch (format)
		{

		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::DeserializeTypeID(out_type_id, location, member_info, index);
			break;
		}

		}


	}

	static bool CheckValidMemberPointer(void* location, void* member_info, uint16_t format)
	{

		switch (format)
		{

		case SerializationFormat::YAML:
		{
			return YAMLTypeSerializer::CheckValidMemberPointer(location, member_info);
			break;
		}

		}

		return true;
	}

	static void GetParentLocation(void* location, char* out_location, void* member_info, uint16_t format)
	{
		switch (format)
		{
		case SerializationFormat::YAML:
		{
			YAMLTypeSerializer::GetParentLocation(location, out_location, member_info);
			break;
		}
		}
	}

	// ==============================================================================

	template<typename T>
	void DeserializeContainer(T& object, void* location, void* member_info, uint16_t format)
	{
		throw std::runtime_error("No definition found");
	}

	template<typename T>
	void Deserialize(T& object, void* location, void* member_info, uint16_t format)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			if (object != nullptr)
			{
				TypeInfo& type_info = TypeRegistry::GetTypesData().data[object->GetTypeID()];
				type_info.deserializer(object, location, member_info, format);
			}

			return;
		}

		constexpr uint64_t type_id = TypeID<T>();
		if (!TypeRegistry::HasType(type_id))
		{
			std::cout << "Type has not registered ->" << TypeName<T>() << std::endl;
			return;
		}

		TypeInfo& info = TypeRegistry::GetTypesData().data[type_id];

		

		if (info.IsContainer() || info.IsKeyValueContainer())
		{
			DeserializeContainer(object, location, member_info, format);
			return;
		}

		if (TypeRegistry::HasMembers(type_id))
		{
			uint32_t index = 0;
			void* type_location = location;
			char _location[128] = { 0 };
			GetTypeMemberLocation(type_id, info.name, location, _location, member_info, format);

			if (member_info)
			{
				Member& class_info = *(Member*)member_info;
				if (class_info.variable.IsPointer())
				{
					//memcpy(type_location, &location, sizeof(void*));
				}
			}
			else
			{
				type_location = (char*)_location;
			}

			if (info.HasParent())
			{
				auto it = TypeRegistry::GetTypesData().data.find(info.parent_class);

				if (it != TypeRegistry::GetTypesData().data.end())
				{
					char parent_location[128] = { 0 };
					GetParentLocation(type_location, parent_location, member_info, format);
					it->second.deserializer(&object, parent_location, nullptr, format);
				}

			}

			

			for (auto& member : TypeRegistry::GetTypesData().members_data[type_id])
			{
				void* member_location = (char*)&object + member.offset;
				TypeInfo& member_type_info = TypeRegistry::GetTypesData().data[member.type_id];
				char member_data_location[128] = { 0 };
				GetMemberLocation(type_location, member_data_location, (void*)&member, format);
				member_type_info.deserializer(member_location, member_data_location, (void*)&member, format);

				index++;

			}
		}
		else if (!TypeRegistry::HasMembers(type_id) && info.HasParent())
		{
			void* type_location = location;
			char _location[128] = { 0 };
			GetTypeMemberLocation(type_id, info.name, location, _location, member_info, format);

			if (member_info)
			{
				Member& class_info = *(Member*)member_info;
				if (class_info.variable.IsPointer())
				{
					//memcpy(type_location, &location, sizeof(void*));
				}
			}
			else
			{
				type_location = (char*)_location;
			}
			if (info.HasParent())
			{
				auto it = TypeRegistry::GetTypesData().data.find(info.parent_class);

				if (it != TypeRegistry::GetTypesData().data.end())
				{
					char parent_location[128] = { 0 };
					GetParentLocation(type_location, parent_location, member_info, format);
					it->second.deserializer(&object, parent_location, member_info, format);
				}

			}
		}
		else
		{
			DeserializeTypeData(object, location, member_info, format);
		}


	}

	template<typename T>
	void Deserialize(T*& object, void* location, void* member_info, uint16_t format)
	{
		if (object != nullptr)
		{
			Deserialize(*object, location, member_info, format);
		}
		else if (member_info && (object == nullptr))
		{

			bool is_valid_pointer = CheckValidMemberPointer(location, member_info, format);
			if (!is_valid_pointer)
			{				
				return;
			}
			uint64_t type_id = 0;
			char type_id_location[128] = { 0 };
			//GetTypeMemberLocation(type_id, "", location)
			DeserializeTypeID(type_id, location, member_info, format);
			if (!TypeRegistry::HasType(type_id))
			{
				return;
			}
			TypeInfo& info = TypeRegistry::GetTypesData().data[type_id];
			void* data = info.construct();
			if (!data)
			{
				TRC_WARN("Can't construct type:{}, Funtion: {}", info.name, __FUNCTION__);
				return;
			}
			object = (T*)data;
			info.deserializer(&object, location, member_info, format);
		}
	}

	template<typename T>
	void Deserialize(Ref<T>& object, void* location, void* member_info, uint16_t format)
	{
		if constexpr (std::is_base_of_v<Resource, T>)
		{
			UUID id = 0;
			Deserialize(id, location, member_info, format);
			if (id == 0)
			{
				return;
			}
			std::string path = GetPathFromUUID(id).string();
			object = T::Deserialize(path);
		}

	}

	template<>
	void Deserialize<StringID>(StringID& object, void* location, void* member_info, uint16_t format);

	template<typename T, typename Alloc>
	void DeserializeContainer(std::vector<T, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		void* type_location = location;

		char _location[128] = { 0 };
		if (!member_info)
		{
			constexpr uint64_t container_id = TypeID< std::vector<T, Alloc>>();
			TypeInfo& container_info = TypeRegistry::GetTypesData().data[container_id];
			GetTypeMemberLocation(container_id, container_info.name, location, _location, nullptr, format);
			type_location = (char*)_location;
		}
		uint32_t size = 0;
		DeserializeContainerSize(size, type_location, member_info, format);
		uint32_t index = 1;

		if (size <= 0)
		{
			return;
		}

		object.reserve(size);

		using type = remove_all_pointers_t<T>;
		uint64_t type_id = TypeID<type>();
		TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

		for (uint32_t i = index; i < (size + 1); i++)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				DeserializeTypeID(type_id, type_location, nullptr, format, index);
				t_info = TypeRegistry::GetTypesData().data[type_id];
				++index;
				type* member = (type*)t_info.construct();
				DeserializeContainerMember(*member, index, t_info, type_location, member_info, format);
				object.emplace_back(member);
			}
			else
			{
				T member{};
				DeserializeContainerMember(member, i, t_info, type_location, member_info, format);
				object.emplace_back(member);
			}
			index++;
		}

	}

	template<typename T, typename Compare, typename Alloc>
	void DeserializeContainer(std::set<T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		void* type_location = location;

		char _location[128] = { 0 };
		if (!member_info)
		{
			constexpr uint64_t container_id = TypeID< std::set<T, Compare, Alloc>>();
			TypeInfo& container_info = TypeRegistry::GetTypesData().data[container_id];
			GetTypeMemberLocation(container_id, container_info.name, location, _location, nullptr, format);
			type_location = (char*)_location;
		}
		uint32_t size = 0;
		DeserializeContainerSize(size, type_location, member_info, format);
		uint32_t index = 1;

		if (size <= 0)
		{
			return;
		}

		using type = remove_all_pointers_t<T>;
		uint64_t type_id = TypeID<type>();

		TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

		for (uint32_t i = index; i < (size + 1); i++)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				DeserializeTypeID(type_id, type_location, nullptr, format, index);
				++index;
				t_info = TypeRegistry::GetTypesData().data[type_id];
				type* member = (type*)t_info.construct();
				DeserializeContainerMember(*member, index, t_info, type_location, member_info, format);
				object.emplace(member);
			}
			else
			{
				T member{};
				DeserializeContainerMember(member, i, t_info, type_location, member_info, format);
				object.emplace(member);
			}
			index++;
		}
	}

	template<typename T, typename Compare, typename Alloc>
	void DeserializeContainer(std::unordered_set<T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		void* type_location = location;

		char _location[128] = { 0 };
		if (!member_info)
		{
			constexpr uint64_t container_id = TypeID< std::unordered_set<T, Compare, Alloc>>();
			TypeInfo& container_info = TypeRegistry::GetTypesData().data[container_id];
			GetTypeMemberLocation(container_id, container_info.name, location, _location, nullptr, format);
			type_location = (char*)_location;
		}
		uint32_t size = 0;
		DeserializeContainerSize(size, type_location, member_info, format);
		uint32_t index = 1;

		if (size <= 0)
		{
			return;
		}

		using type = remove_all_pointers_t<T>;
		uint64_t type_id = TypeID<type>();
		
		TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

		for (uint32_t i = index; i < (size + 1); i++)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				DeserializeTypeID(type_id, type_location, nullptr, format, index);
				++index;
				t_info = TypeRegistry::GetTypesData().data[type_id];
				type* member = (type*)t_info.construct();
				DeserializeContainerMember(*member, index, t_info, type_location, member_info, format);
				object.emplace(member);
			}
			else
			{
				T member{};
				DeserializeContainerMember(member, i, t_info, type_location, member_info, format);
				object.emplace(member);
			}
			index++;
		}
	}

	template<typename Key, typename T, typename Compare, typename Alloc>
	void DeserializeContainer(std::map<Key, T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		using key_type = std::remove_cv_t<Key>;

		void* type_location = location;

		char _location[128] = { 0 };
		if (!member_info)
		{
			constexpr uint64_t container_id = TypeID<std::map<Key, T, Compare, Alloc>>();
			TypeInfo& container_info = TypeRegistry::GetTypesData().data[container_id];
			GetTypeMemberLocation(container_id, container_info.name, location, _location, nullptr, format);
			type_location = (char*)_location;
		}
		uint32_t size = 0;
		DeserializeContainerSize(size, type_location, member_info, format);
		uint32_t index = 1;

		if (size <= 0)
		{
			return;
		}

		using type = remove_all_pointers_t<T>;
		uint64_t type_id = TypeID<type>();
		
		TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

		for (uint32_t i = index; i < (size + 1); i++)
		{
			key_type key_value{};

			if constexpr (std::is_pointer_v<T>)
			{
				DeserializeTypeID(type_id, type_location, nullptr, format, index);
				++index;
				t_info = TypeRegistry::GetTypesData().data[type_id];
				type* value_data = (type*)t_info.construct();
				DeserializeKeyValuePair(key_value, *value_data, t_info, index, type_location, member_info, format);

				object[key_value] = value_data;
			}
			else
			{
				T value_data{};

				DeserializeKeyValuePair(key_value, value_data, t_info, index, type_location, member_info, format);

				object[key_value] = value_data;
			}
			index++;
		}
	}

	template<typename Key, typename T, typename Compare, typename Alloc>
	void DeserializeContainer(std::unordered_map<Key, T, Compare, Alloc>& object, void* location, void* member_info, uint16_t format)
	{
		using key_type = std::remove_cv_t<Key>;

		void* type_location = location;

		char _location[128] = { 0 };
		if (!member_info)
		{
			constexpr uint64_t container_id = TypeID<std::unordered_map<Key, T, Compare, Alloc>>();
			TypeInfo& container_info = TypeRegistry::GetTypesData().data[container_id];
			GetTypeMemberLocation(container_id, container_info.name, location, _location, nullptr, format);
			type_location = (char*)_location;
		}
		uint32_t size = 0;
		DeserializeContainerSize(size, type_location, member_info, format);
		uint32_t index = 1;

		if (size <= 0)
		{
			return;
		}

		using type = remove_all_pointers_t<T>;
		uint64_t type_id = TypeID<type>();

		TypeInfo t_info = TypeRegistry::GetTypesData().data[type_id];

		for (uint32_t i = index; i < (size + 1); i++)
		{
			key_type key_value{};

			if constexpr (std::is_pointer_v<T>)
			{
				DeserializeTypeID(type_id, type_location, nullptr, format, index);
				++index;
				t_info = TypeRegistry::GetTypesData().data[type_id];
				type* value_data = (type*)t_info.construct();
				DeserializeKeyValuePair(key_value, *value_data, t_info, index, type_location, member_info, format);

				object[key_value] = value_data;
			}
			else
			{
				T value_data{};

				DeserializeKeyValuePair(key_value, value_data, t_info, index, type_location, member_info, format);

				object[key_value] = value_data;
			}
			index++;
		}
	}

	// -----------------------------------------------------------------------------------

}
