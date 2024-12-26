#pragma once

#include "TypeHash.h"
#include "TypeRegistry.h"

#include <fstream>
#include <iostream>

namespace trace::Reflection {

	template<typename T>
	void Serialize(T& object, void* location, void* member_info, uint16_t format);

	template<typename T>
	void Serialize(T*& object, void* location, void* member_info, uint16_t format);

	template<typename T>
	void SerializeContainer(T& object, void* location, void* member_info, uint16_t format);

	template<typename T>
	void Deserialize(T& object, void* location, void* member_info, uint16_t format);

	template<typename T>
	void Deserialize(T*& object, void* location, void* member_info, uint16_t format);

	template<typename T>
	void DeserializeContainer(T& object, void* location, void* member_info, uint16_t format);


	
	// Helper function template to test `std::ostream <<`
	template <typename T>
	auto test_ostream_operator(std::ostream& os, const T& obj) -> decltype(os << obj, void(), std::true_type{});

	std::false_type test_ostream_operator(...); // Fallback for invalid types

	// Check if a type has `std::ostream <<`
	template <typename T>
	constexpr bool has_ostream_operator_v = decltype(test_ostream_operator(std::declval<std::ostream&>(), std::declval<const T&>()))::value;

	class ConsoleSerializer
	{

	public:



		template<typename T>
		static void SerializeTypeData(T& obj)
		{
			//std::ostream::operator<<;

			if constexpr (has_ostream_operator_v<T>)
			{
				std::cout << obj << " ";
			}
		}

		static void SerializeContainerSize(uint32_t size)
		{
			std::cout << "Size: " << size << "\n";
		}

		static void BeginContainer()
		{
			std::cout << "[ ";
		}
		static void EndContainer()
		{
			std::cout << " ]";
		}

		static void BeginKeyValueContainer()
		{
			std::cout << "{ " << std::endl;
		}
		static void EndKeyValueContainer()
		{
			std::cout << std::endl << " }" << std::endl;
		}

		static void BeginTypeMembers(uint64_t type_id, std::string_view type_name)
		{
			std::cout << type_name << " ( " << std::endl;
		}

		static void EndTypeMembers(uint64_t type_id, std::string_view type_name)
		{
			std::cout << std::endl << " )" << std::endl;
		}


		template<typename Key, typename Value>
		static void SerializeKeyValuePair(Key& key, Value& value)
		{
			Serialize(key, nullptr, nullptr, 0);
			std::cout << ": ";
			Serialize(value, nullptr, nullptr, 0);
		}

	};

	class BinarySerializer
	{

	public:
		template<typename T>
		static void SerializeTypeData(T& obj, void* location)
		{

			std::fstream& stream = *(static_cast<std::fstream*>(location));

			stream.write((const char*)&obj, sizeof(T));

		}

		static void SerializeContainerSize(uint32_t size, void* location)
		{
			std::fstream& stream = *(static_cast<std::fstream*>(location));

			uint32_t* _size = &size;
			stream.write((const char*)_size, sizeof(uint32_t));
		}

		static void BeginContainer(void* location)
		{

		}
		static void EndContainer(void* location)
		{
		}

		static void BeginKeyValueContainer(void* location)
		{
		}
		static void EndKeyValueContainer(void* location)
		{
		}

		static void BeginTypeMembers(uint64_t type_id, std::string_view type_name, void* location)
		{
		}

		static void EndTypeMembers(uint64_t type_id, std::string_view type_name, void* location)
		{
		}


		template<typename Key, typename Value>
		static void SerializeKeyValuePair(Key& key, Value& value, void* location)
		{
			SerializeTypeData(key, location);
			SerializeTypeData(value, location);
		}

		template<typename T>
		static void DeserializeTypeData(T& obj, void* location)
		{

			std::fstream& stream = *(static_cast<std::fstream*>(location));

			stream.read((char*)&obj, sizeof(T));

		}

		static void DeserializeContainerSize(uint32_t& out_size, void* location)
		{
			out_size = 0;

			std::fstream& stream = *(static_cast<std::fstream*>(location));

			stream.read((char*)&out_size, sizeof(uint32_t));
		}

		template<typename T>
		static void DeserializeContainerMember(T& obj, uint32_t index, void* location)
		{
			DeserializeTypeData(obj, location);
		}

		template<typename Key, typename Value>
		static void DeserializeKeyValuePair(Key& key, Value& value, uint32_t index, void* location)
		{
			DeserializeTypeData(key, location);
			DeserializeTypeData(value, location);
		}

	};

}
