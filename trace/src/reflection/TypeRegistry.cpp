#include "pch.h"

#include "Variable.h"
#include "core/Coretypes.h"
#include "SerializeTypes.h"

namespace trace::Reflection {
	bool operator==(const Variable& lhs, const Variable& rhs)
	{
		return	lhs.m_typeId == rhs.m_typeId &&
			lhs.m_arraySize == rhs.m_arraySize &&
			lhs.m_pointerAmount == rhs.m_pointerAmount &&
			lhs.m_traitFlags == rhs.m_traitFlags;
	}

	template<>
	void Serialize<StringID>(StringID& object, void* location, void* member_info, uint16_t format)
	{
		std::string& str = STRING_FROM_ID(object);
		Serialize(str, location, member_info, format);

	}

	template<>
	void Deserialize<StringID>(StringID& object, void* location, void* member_info, uint16_t format)
	{
		std::string val;
		Deserialize(val, location, member_info, format);
		object = STR_ID(val);
	}
}