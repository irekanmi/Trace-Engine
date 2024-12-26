#pragma once

#include "TypeHash.h"
#include <type_traits>
#include <unordered_map>
#include <typeinfo>
#include "Type.h"
#include <string>

namespace trace::Reflection {


	class Variable final
	{

	public:

		static constexpr uint32_t ConstFlag = 1 << 0;
		static constexpr uint32_t ReferenceFlag = 1 << 1;
		static constexpr uint32_t VolatileFlag = 1 << 2;
		static constexpr uint32_t RValReferenceFlag = 1 << 3;

	public:

		constexpr explicit Variable(uint64_t id) : m_typeId{ id } {};
		constexpr Variable() = default;

		template <typename T>
		static constexpr Variable Create();


	public:

		constexpr void SetTypeId(uint64_t id) { m_typeId = id; }

		constexpr void SetConstFlag() { m_traitFlags |= ConstFlag; }
		constexpr void SetReferenceFlag() { m_traitFlags |= ReferenceFlag; }
		constexpr void SetVolatileFlag() { m_traitFlags |= VolatileFlag; }
		constexpr void SetRValReferenceFlag() { m_traitFlags |= RValReferenceFlag; }

		constexpr void RemoveConstFlag() { m_traitFlags &= ~ConstFlag; }
		constexpr void RemoveReferenceFlag() { m_traitFlags &= ~ReferenceFlag; }
		constexpr void RemoveVolatileFlag() { m_traitFlags &= ~VolatileFlag; }
		constexpr void RemoveRValReferenceFlag() { m_traitFlags &= ~RValReferenceFlag; }

		constexpr void SetPointerAmount(uint16_t amount) { m_pointerAmount = amount; }
		constexpr uint32_t GetPointerAmount() const { return m_pointerAmount; }

		constexpr void SetArraySize(uint32_t Size) { m_arraySize = Size; }
		constexpr uint32_t GetArraySize() const { return m_arraySize; }

		constexpr bool IsConst() const { return m_traitFlags & ConstFlag; }
		constexpr bool IsReference() const { return m_traitFlags & ReferenceFlag; }
		constexpr bool IsVolatile() const { return m_traitFlags & VolatileFlag; }
		constexpr bool IsRValReference() const { return m_traitFlags & RValReferenceFlag; }
		constexpr bool IsPointer() const { return m_pointerAmount; }
		constexpr bool IsArray() const { return m_arraySize == 1; }
		constexpr bool IsRefOrPointer() const { return IsPointer() || IsReference() || IsRValReference(); }


		constexpr uint64_t	GetHash() const { return m_typeId ^ m_arraySize ^ (static_cast<uint64_t>(m_pointerAmount) << 32) ^ (static_cast<uint64_t>(m_traitFlags) << 40); }


		constexpr uint64_t GetTypeID() const { return m_typeId; }

		friend bool operator==(const Variable& lhs, const Variable& rhs);


	private:

		uint64_t m_typeId{ };	// The underlying type id
		uint32_t m_arraySize{ };	// if the variable is a fixed sized array, the size will be contained in this. else it will be 1
		uint16_t m_pointerAmount{ };	// The amount of pointers that are attached to the Type
		uint8_t	m_traitFlags{ };	// Other flags (const, volatile, reference, RValReference)

	protected:

	};

	


	template<typename T>
	inline constexpr Variable Variable::Create()
	{
		using Type_RemovedExtents = std::remove_all_extents_t<T>;
		using Type_RemovedRefs = std::remove_reference_t<Type_RemovedExtents>;
		using Type_RemovedPtrs = remove_all_pointers_t<Type_RemovedRefs>;

		using StrippedType = std::remove_cv_t<Type_RemovedPtrs>;
		RegisterTypeObject<StrippedType> TypeRegister{};

		constexpr bool IsRef{ std::is_reference_v<T> };
		constexpr bool IsRValRef{ std::is_rvalue_reference_v<T> };
		constexpr bool IsConst{ std::is_const_v<Type_RemovedPtrs> };
		constexpr bool IsVolatile{ std::is_volatile_v<Type_RemovedPtrs> };

		constexpr uint32_t PointerAmount{ count_pointers<Type_RemovedRefs>() };

		auto variable = Variable(TypeID<StrippedType>());

		if constexpr (IsConst)
		{
			variable.SetConstFlag();
		}
		if constexpr (IsVolatile)
		{
			variable.SetVolatileFlag();
		}
		if constexpr (IsRef)
		{
			variable.SetReferenceFlag();
		}
		if constexpr (IsRValRef)
		{
			variable.SetRValReferenceFlag();
		}

		variable.SetPointerAmount(PointerAmount);

		if constexpr (!std::is_same_v<void, Type_RemovedExtents>)
		{
			constexpr uint32_t ArraySize{ sizeof(T) / sizeof(Type_RemovedExtents) };
			variable.SetArraySize(ArraySize);
		}
		else
		{
			variable.SetArraySize(1);
		}

		return variable;
	}


}

template <typename T>
constexpr uint32_t count_pointers(uint32_t counter = 0)
{
	if constexpr (std::is_pointer_v<T>)
		return count_pointers<std::remove_pointer_t<T>>(++counter);
	else
		return counter;
}

//https://stackoverflow.com/questions/9851594/standard-c11-way-to-remove-all-pointers-of-a-type

template <typename T>
struct identity_t
{
	using type = T;
};

template<typename T>
struct remove_all_pointers : std::conditional_t<
	std::is_pointer_v<T>,
	remove_all_pointers<
	std::remove_pointer_t<T>
	>,
	identity_t<T>
>
{};

template<typename T>
using remove_all_pointers_t = typename remove_all_pointers<T>::type;

template <>
struct std::hash<trace::Reflection::Variable>
{
	std::size_t operator()(const trace::Reflection::Variable& id) const noexcept
	{
		return static_cast<size_t>(id.GetHash());
	}
};


