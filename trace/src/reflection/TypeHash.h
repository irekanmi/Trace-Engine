#pragma once

#include <string_view>
#include <functional>
#include <array>

namespace trace::Reflection {

	enum SerializationFormat
	{
		UNKNOWN,
		JSON,
		YAML,
		BINARY
	};

	template<typename T> constexpr std::string_view TypeName();

	template<>
	constexpr std::string_view TypeName<void>()
	{
		return "void";
	}

	namespace detail {

		using type_name_prober = void;

		template<typename T>
		constexpr std::string_view wrapper_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#endif
		}

		constexpr std::size_t wrapper_type_name_prefix_length()
		{
			return wrapper_type_name<type_name_prober>().find(TypeName<type_name_prober>());
		}

		constexpr std::size_t wrapper_type_name_suffix_length()
		{
			return wrapper_type_name<type_name_prober>().length()
				- wrapper_type_name_prefix_length()
				- TypeName<type_name_prober>().length();
		}
	}

	template<typename T>
	constexpr std::string_view TypeName()
	{
		constexpr auto wrapped_name = detail::wrapper_type_name<T>();
		constexpr auto prefix_length = detail::wrapper_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapper_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;

		return wrapped_name.substr(prefix_length, type_name_length);
	}



	//Hashing
	constexpr uint64_t hash(std::string_view str)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (char c : str)
		{
			hash_value ^= static_cast<std::uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}


	template<typename T>
	constexpr uint64_t TypeID()
	{
		using type = std::remove_reference_t<T>;
		return hash(TypeName<type>());
	}

}
