#pragma once

#include "reflection/TypeHash.h"
#include "reflection/Type.h"
#include "core/io/Logging.h"
#include "reflection/Member.h"
#include "serialize/DataStream.h"

#include <cinttypes>
#include <type_traits>

#define PTR_INVALID 0xFF0000FF000000FF

namespace trace {

	namespace Reflection {
		template<typename T>
		void Serialize(T& object, void* location, void* member_info, uint16_t format);

		template<typename T>
		void Deserialize(T& object, void* location, void* member_info, uint16_t format);

	}



	class BinaryTypeSerializer
	{

	public:

		template<typename T>
		static void SerializeTypeData(T& obj, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;

			if constexpr (std::is_same_v<T, std::string>)
			{				

				std::string& str = static_cast<std::string&>(obj);

				size_t str_size = str.length() + 1;
				stream->Write<size_t>(str_size);
				stream->Write(str.data(), static_cast<uint32_t>(str_size));
			}
			else if constexpr (std::is_same_v<T, char>)
			{
				if (member_info)
				{
					Reflection::Member& mem = *(Reflection::Member*)member_info;
					stream->Write((void*)&obj, mem.variable.GetArraySize());
				}
				else
				{
					std::string val(&obj);
					SerializeTypeData(val, location, member_info);
				}
			}
			else if constexpr (Reflection::IsTypeContainer<T>{} || Reflection::IsTypeKeyValueContainer<T>{})
			{
				return;
			}
			else if constexpr (!std::is_pointer_v<T>)
			{
				stream->Write<T>(obj);
			}




		}

		static void SerializeContainerSize(uint32_t size, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
			stream->Write<uint32_t>(size);
		}

		static void SerializeTypeID(uint64_t type_id, void* location, void* member_info, int32_t index = -1)
		{
			DataStream* stream = (DataStream*)location;
			stream->Write<uint64_t>(type_id);
		}

		static void SerializeNullMember(void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
			uintptr_t null_member = PTR_INVALID;
			stream->Write<uintptr_t>(null_member);
		}

		template<typename T>
		static void SerializeContainerMember(T& obj, uint32_t index, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			type_info.serializer(&obj, location, nullptr, (uint16_t)Reflection::SerializationFormat::BINARY);
			
		}

		static void BeginContainer(std::string_view type_name, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;

		}
		static void EndContainer(void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
		}

		static void BeginKeyValueContainer(std::string_view type_name, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
		}
		static void EndKeyValueContainer(void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
		}

		static void BeginTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
		}

		static void EndTypeMembers(uint64_t type_id, std::string_view type_name, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;

		}

		static void BeginParent(void* location)
		{
			DataStream* stream = (DataStream*)location;
		}

		static void EndParent(void* location)
		{
			DataStream* stream = (DataStream*)location;
		}

		static void GetParentLocation(void* location, char* out_location, void* member_info)
		{

		}


		template<typename Key, typename Value>
		static void SerializeKeyValuePair(Key& key, Value& value, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
			Reflection::Serialize(key, location, nullptr, Reflection::SerializationFormat::BINARY);
			type_info.serializer(&value, location, nullptr, (uint16_t)Reflection::SerializationFormat::BINARY);


		}

		static void GetMemberLocation(void* location, char* out_location, void* member_info)
		{
			TRC_ASSERT(member_info, "Invalid Member Info, Funtion: {}", __FUNCTION__);
			

		}
		static void GetTypeMemberLocation(uint64_t type_id, std::string_view type_name, void* location, char* out_location, void* member_info)
		{
			
		}

		static bool CheckValidMemberPointer(void* location, void* member_info)
		{
			TRC_ASSERT(member_info, "Invalid Member Info, Funtion: {}", __FUNCTION__);
			DataStream* stream = (DataStream*)location;
			uintptr_t null_member = 0;
			stream->Read<uintptr_t>(null_member);

			if (null_member == PTR_INVALID)
			{
				return false;
			}
			uint32_t curr_pos = stream->GetPosition();
			curr_pos -= sizeof(uintptr_t);
			stream->SetPosition(curr_pos);
			return true;
		}

		template<typename T>
		static void DeserializeTypeData(T& obj, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;

			if constexpr (std::is_same_v<T, std::string>)
			{
				std::string& str = static_cast<std::string&>(obj);

				size_t str_size;
				stream->Read<size_t>(str_size);
				str.reserve(str_size);
				str.resize(str_size - 1);
				stream->Read(str.data(), static_cast<uint32_t>(str_size));
			}
			else if constexpr (std::is_same_v<T, char>)
			{
				if (member_info)
				{
					Reflection::Member& mem = *(Reflection::Member*)member_info;
					stream->Read((void*)&obj, mem.variable.GetArraySize());
				}
				else
				{
					size_t str_size;
					stream->Read<size_t>(str_size);
					stream->Read(&obj, static_cast<uint32_t>(str_size));

				}
			}
			else if constexpr (!std::is_pointer_v<T>)
			{
				stream->Read<T>(obj);
			}

		}

		static void DeserializeContainerSize(uint32_t& out_size, void* location, void* member_info)
		{
			DataStream* stream = (DataStream*)location;
			stream->Read<uint32_t>(out_size);
		}

		static void DeserializeTypeID(uint64_t& out_type_id, void* location, void* member_info, int32_t index = -1)
		{
			DataStream* stream = (DataStream*)location;
			stream->Read<uint64_t>(out_type_id);
		}

		template<typename T>
		static void DeserializeContainerMember(T& obj, uint32_t index, Reflection::TypeInfo& type_info, void* location, void* member_info)
		{
			type_info.deserializer(&obj, location, nullptr, (uint16_t)Reflection::SerializationFormat::BINARY);
		}

		template<typename Key, typename Value>
		static void DeserializeKeyValuePair(Key& key, Value& value, Reflection::TypeInfo& type_info, uint32_t index, void* location, void* member_info)
		{
			Reflection::Deserialize(key, location, nullptr, Reflection::SerializationFormat::BINARY);
			type_info.deserializer(&value, location, nullptr, (uint16_t)Reflection::SerializationFormat::BINARY);

		}

	private:
	protected:

	};

}
