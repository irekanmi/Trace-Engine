#include "pch.h"

#include "Variable.h"

namespace trace::Reflection {
	bool operator==(const Variable& lhs, const Variable& rhs)
	{
		return	lhs.m_typeId == rhs.m_typeId &&
			lhs.m_arraySize == rhs.m_arraySize &&
			lhs.m_pointerAmount == rhs.m_pointerAmount &&
			lhs.m_traitFlags == rhs.m_traitFlags;
	}
}