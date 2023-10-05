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

}
