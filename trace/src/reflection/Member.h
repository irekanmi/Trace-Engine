#pragma once

#include "TypeHash.h"
#include "Variable.h"

#include <string>
#include <bitset>

namespace trace::Reflection {

	struct Member final
	{

		uint32_t offset{};
		uint32_t size{};
		uint32_t align{};
		std::string name{};
		Variable variable{};
		uint64_t type_id{};

		std::bitset<128> serialization_flags;// NOTE: Used as custom serialiazation flags for class members


		constexpr Member() = default;
		~Member() = default;
		constexpr Member(const Member&) = default;
		constexpr Member& operator=(const Member&) = default;
		constexpr Member(Member&&) noexcept = default;
		constexpr Member& operator=(Member&&) noexcept = default;

		constexpr bool operator<(const Member& rhs) const
		{
			return offset < rhs.offset;
		}
	};

}
