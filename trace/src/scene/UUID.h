#pragma once

#include <xhash>

namespace trace {

	class UUID
	{

	public:
		UUID();
		UUID(uint64_t val) : m_val(val) {}
		UUID(const UUID&) = default;

		static UUID GenUUID();

		operator uint64_t() const { return m_val; }
		operator uint64_t(){ return m_val; }
	private:
		uint64_t m_val = 0;

	protected:

	};

	class UUID_32
	{

	public:
		UUID_32();
		UUID_32(uint32_t val) : m_val(val) {}
		UUID_32(const UUID_32&) = default;

		static UUID_32 GenUUID_32();

		operator uint32_t() const { return m_val; }
		operator uint32_t() { return m_val; }
	private:
		uint32_t m_val = 0;

	protected:

	};

}


namespace std {

	template<>
	struct hash<trace::UUID>
	{

		std::size_t operator()(const trace::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}

	};

	template<>
	struct hash<trace::UUID_32>
	{

		size_t operator()(const trace::UUID_32& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}

	};

}
