#pragma once

#include "TypeHash.h"
#include "Container.h"
#include <string_view>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <unordered_set>

namespace trace::Reflection {

	template<typename T>
	struct IsTypeContainer : std::false_type
	{};

	template<typename T, typename Alloc>
	struct IsTypeContainer<std::vector<T, Alloc>> : std::true_type
	{};

	template<typename T, typename Compare, typename Alloc>
	struct IsTypeContainer<std::set<T, Compare, Alloc>> : std::true_type
	{};

	template<typename T, typename Compare, typename Alloc>
	struct IsTypeContainer<std::unordered_set<T, Compare, Alloc>> : std::true_type
	{};

	template<typename T>
	struct IsTypeKeyValueContainer : std::false_type
	{};

	template<typename Key, typename T, typename Compare, typename Alloc>
	struct IsTypeKeyValueContainer<std::map<Key, T, Compare, Alloc>> : std::true_type
	{};

	template<typename Key, typename T, typename Compare, typename Alloc>
	struct IsTypeKeyValueContainer<std::unordered_map<Key, T, Compare, Alloc>> : std::true_type
	{};

	template<typename T>
	struct ContainerTraits
	{};

	template<typename T, typename Alloc>
	struct ContainerTraits<std::vector<T, Alloc>>
	{
		using type = T;
	};

	template<typename T, typename Compare, typename Alloc>
	struct ContainerTraits<std::set<T, Compare, Alloc>>
	{
		using type = T;
	};

	template<typename T, typename Compare, typename Alloc>
	struct ContainerTraits<std::unordered_set<T, Compare, Alloc>>
	{
		using type = T;
	};

	template<typename T>
	struct KeyValueContainerTraits
	{};

	template<typename Key, typename T, typename Compare, typename Alloc>
	struct KeyValueContainerTraits<std::map<Key, T, Compare, Alloc>>
	{
		using key_type = Key;
		using value_type = T;
	};

	template<typename Key, typename T, typename Compare, typename Alloc>
	struct KeyValueContainerTraits<std::unordered_map<Key, T, Compare, Alloc>>
	{
		using key_type = Key;
		using value_type = T;
	};


	struct TypeInfo final
	{
		uint64_t parent_class{};
		std::string_view name;
		uint32_t size;
		uint32_t alignment;
		Reflection::Container container_info{};
		std::function<void(void* data, void* location, void* member_info, uint16_t format)> serializer;// NOTE: Use a static variable to determine format
		std::function<void(void* data, void* location, void* member_info, uint16_t format)> deserializer;// NOTE: Use a static variable to determine format
		std::function<void*()> construct;// NOTE: Use a static variable to determine format


		bool IsContainer()
		{
			return container_info.value_type_id != 0 && container_info.key_type_id == 0;
		}

		bool IsKeyValueContainer()
		{
			return container_info.value_type_id != 0 && container_info.key_type_id != 0;
		}

		bool HasParent()
		{
			return parent_class != 0;
		}




	};



}
