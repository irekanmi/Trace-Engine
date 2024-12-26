#pragma once

#include "Type.h"
#include "TypeHash.h"
#include "Member.h"
#include "Variable.h"
#include "Serialize.h"
#include <set>

namespace trace::Reflection {

	template<typename T>
	class RegisterType;


	class TypeRegistry
	{

	public:

		struct TypesData
		{
			std::unordered_map<uint64_t, TypeInfo> data{};
			std::unordered_map<uint64_t, std::set<Member>> members_data{};
		};

		static bool HasType(uint64_t id)
		{
			return GetTypesData().data.find(id) != GetTypesData().data.end();
		}

		static bool HasMembers(uint64_t id)
		{
			return GetTypesData().members_data.find(id) != GetTypesData().members_data.end();
		}

		template<typename T>
		constexpr static TypeInfo CreateTypeInfo(uint64_t parent_class = 0)
		{
			TypeInfo info{};

			info.parent_class = parent_class;

			info.name = TypeName<T>();
			info.size = sizeof(T);
			info.alignment = alignof(T);

			Container container{};
			container.key_type_id = 0;
			container.value_type_id = 0;

			info.construct = [] () -> void* {
				if constexpr (std::is_constructible_v<T>)
				{
					return new T;// TODO: Use custom allocator
				}
				else
				{
					return nullptr;
				}
			};

			if constexpr (IsTypeContainer<T>{})
			{
				RegisterTypeObject<ContainerTraits<T>::type> value_reg;
				container.value_type_id = TypeID<ContainerTraits<T>::type>();

				info.serializer = [](void* data, void* location, void* member_info, uint16_t format) { SerializeContainer(*static_cast<T*>(data), location, member_info, format); };
				info.deserializer = [](void* data, void* location, void* member_info, uint16_t format) { DeserializeContainer(*static_cast<T*>(data), location, member_info, format); };
			}
			else if constexpr (IsTypeKeyValueContainer<T>())
			{
				RegisterTypeObject<KeyValueContainerTraits<T>::key_type> key_reg;
				RegisterTypeObject<KeyValueContainerTraits<T>::value_type> value_reg;
				container.key_type_id = TypeID<KeyValueContainerTraits<T>::key_type>();
				container.value_type_id = TypeID<KeyValueContainerTraits<T>::value_type>();

				info.serializer = [](void* data, void* location, void* member_info, uint16_t format) { SerializeContainer(*static_cast<T*>(data), location, member_info, format); };
				info.deserializer = [](void* data, void* location, void* member_info, uint16_t format) { DeserializeContainer(*static_cast<T*>(data), location, member_info, format); };
			}
			else
			{
				info.serializer = [](void* data, void* location, void* member_info, uint16_t format) 
				{
					if (!member_info)
					{
						Serialize(*static_cast<T*>(data), location, member_info, format);
					}
					else
					{
						Member& member = *(Member*)member_info;
						if (member.variable.IsPointer())
						{
							Serialize(*static_cast<T**>(data), location, member_info, format);
						}
						else
						{
							Serialize(*static_cast<T*>(data), location, member_info, format);
						}
					}
				};
				info.deserializer = [](void* data, void* location, void* member_info, uint16_t format) 
				{ 
					if (!member_info)
					{
						Deserialize(*static_cast<T*>(data), location, member_info, format);
					}
					else
					{
						Member& member = *(Member*)member_info;
						if (member.variable.IsPointer())
						{
							Deserialize(*static_cast<T**>(data), location, member_info, format);
						}
						else
						{
							Deserialize(*static_cast<T*>(data), location, member_info, format);
						}
					}
				};
			}



			info.container_info = container;

			return info;
		}

		template<typename T>
		static void RegisterType(uint64_t parent_class = 0)
		{
			constexpr uint64_t id = TypeID<T>();

			GetTypesData().data.emplace(id, CreateTypeInfo<T>(parent_class));

			return;
		}

		static Member& RegisterField(uint64_t class_id, Variable variable, const std::string& field_name, uint32_t offset, uint32_t size, uint32_t align)
		{
			Member info{};
			info.name = field_name;
			info.variable = variable;
			info.offset = offset;
			info.size = size;
			info.align = align;
			info.type_id = variable.GetTypeID();

			GetTypesData().members_data[class_id].emplace(info);


			return info;
		}

		static TypesData& GetTypesData()
		{
			static TypesData types_data;
			return types_data;
		}

	private:



	protected:

	};



	template <typename T>
	class RegisterTypeObject
	{
	private:
		class RegisterTypeOnce
		{
		public:
			RegisterTypeOnce(uint64_t parent_class = 0)
			{
				TypeRegistry::RegisterType<T>(parent_class);
			}
		};
		inline static RegisterTypeOnce Registerer{};
	};

	template <typename T, typename Parent>
	class RegisterTypeParentObject
	{
	private:
		class RegisterTypeParentOnce
		{
		public:
			RegisterTypeParentOnce()
			{
				TypeRegistry::RegisterType<T>(TypeID<Parent>());
			}
		};
		inline static RegisterTypeParentOnce Registerer{};
	};

#define _REGISTER_TYPE_INTERNAL(TYPE, VARNAME) trace::Reflection::RegisterTypeObject<TYPE> VARNAME##TYPE {};
#define REGISTER_TYPE(TYPE) _REGISTER_TYPE_INTERNAL(TYPE, RegisterTypeObject_)
#define REGISTER_TYPE_PARENT(TYPE, PARENT) trace::Reflection::RegisterTypeParentObject<TYPE, PARENT> RegisterTypeParentObject_##TYPE##PARENT{};
#define REGISTER_KEY_VALUE_CONTAINER(CONTAINER, KEY, TYPE) trace::Reflection::RegisterTypeObject<CONTAINER<KEY, TYPE>> RegisterTypeObject_##TYPE {};
#define REGISTER_CONTAINER(CONTAINER, TYPE) trace::Reflection::RegisterTypeObject<CONTAINER<TYPE>> RegisterType_##TYPE {};

	struct RegisterMember final
	{
		RegisterMember(uint64_t class_id, trace::Reflection::Variable variable, const std::string& field_name, uint32_t offset, uint32_t size, uint32_t align)
		{
			TypeRegistry::RegisterField(class_id, variable, field_name, offset, size, align);
		}
	};

#define REGISTER_MEMBER(TYPE, FIELD) inline static trace::Reflection::RegisterMember TYPE##FIELD{trace::Reflection::TypeID<TYPE>(), trace::Reflection::Variable::Create<decltype(TYPE::FIELD)>(), #FIELD, offsetof(TYPE, FIELD), sizeof(decltype(TYPE::FIELD)), alignof(decltype(TYPE::FIELD))};

#define ACCESS_CLASS_MEMBERS(CLASS_NAME) friend struct RegisterClass##CLASS_NAME;
#define GET_TYPE_ID public:	 virtual uint64_t GetTypeID() { return trace::Reflection::TypeID<decltype(*this)>(); }
#define BEGIN_REGISTER_CLASS(CLASS_NAME) struct RegisterClass##CLASS_NAME {
#define END_REGISTER_CLASS };



	/*inline const std::string& GetVariableName(const trace::Reflection::Variable& variableId)
	{
		static std::unordered_map<trace::Reflection::Variable, std::string> VariableNames{};

		auto it = VariableNames.find(variableId);
		if (it != VariableNames.end())
		{
			return it->second;
		}

		{

			std::string Name = std::string(trace::Reflection::TypeRegistry::GetTypesData().data[variableId.GetTypeID()].name);

			if (variableId.IsVolatile()) Name = "volatile " + Name;
			if (variableId.IsConst()) Name = "const " + Name;

			const uint32_t pointerAmount = variableId.GetPointerAmount();
			for (uint32_t i{}; i < pointerAmount; ++i)
			{
				Name += '*';
			}

			if (variableId.GetArraySize() > 1)
			{
				Name += '[';
				Name += std::to_string(variableId.GetArraySize());
				Name += ']';
			}

			if (variableId.IsRValReference()) Name += "&&";
			else if (variableId.IsReference()) Name += '&';

			return VariableNames.emplace(variableId, std::move(Name)).first->second;
		}
	}*/

}
