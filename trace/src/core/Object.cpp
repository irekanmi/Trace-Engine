#include "pch.h"
#include "Object.h"

namespace trace {
	uint32_t g_ObjID = -1;
	std::vector<Object*> g_SystemPtrs;


	Object::Object()
	{
		m_id = ++g_ObjID;
		g_SystemPtrs.emplace_back(this);
	}

	Object::Object(const char* name)
	{
		m_id = ++g_ObjID;
		m_name = name;
		g_SystemPtrs.emplace_back(this);
	}

	Object::~Object()
	{

	}

	const char* Object::GetName()
	{
		return m_name;
	}

}