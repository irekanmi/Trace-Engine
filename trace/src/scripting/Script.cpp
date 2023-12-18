#include "pch.h"

#include "Script.h"
#include "ScriptBackend.h"

namespace trace {

	ScriptInstance::ScriptInstance()
	{
	}

	ScriptInstance::ScriptInstance(const ScriptInstance& other)
	{

		m_script = other.m_script;
		m_internal = other.m_internal;
		m_fields = other.m_fields;

	}

	ScriptInstance::~ScriptInstance()
	{
	}

	bool ScriptInstance::Init()
	{
		if (!m_script) return false;

		for (auto& i : m_script->m_fields)
		{
			
		}

		return true;
	}

	void ScriptInstance::Shutdown()
	{
		m_fields.clear();
	}

	

	bool ScriptInstance::GetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size)
	{
		if (m_script->m_fields.find(field_name) != m_script->m_fields.end())
		{
			return GetInstanceFieldValue(*this, m_script->m_fields.at(field_name), value, val_size);
		}

		return false;
	}

	void ScriptInstance::SetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size)
	{
		if (m_script->m_fields.find(field_name) != m_script->m_fields.end())
		{
			SetInstanceFieldValue(*this, m_script->m_fields.at(field_name), value, val_size);
		}
	}

	Script::Script()
	{
	}

	Script::~Script()
	{
	}

	ScriptInstance Script::CreateInstance()
	{
		ScriptInstance out_instance;
		CreateScriptInstance(*this, out_instance);
		out_instance.Init();
		return out_instance;
	}

	ScriptMethod* Script::GetMethod(const std::string& method)
	{
		if (m_methods.find(method) != m_methods.end())
		{
			return &m_methods.at(method);
		}

		return nullptr;
	}

	uintptr_t Script::GetID()
	{
		uintptr_t res = 0;
		GetScriptID(*this, res);
		return res;
	}

	ScriptMethod::ScriptMethod()
	{
	}

	ScriptMethod::~ScriptMethod()
	{
	}

	ScriptFieldInstance::ScriptFieldInstance()
	{
	}

	ScriptFieldInstance::~ScriptFieldInstance()
	{
	}

	bool ScriptFieldInstance::Init(Script* script)
	{
		m_script = script;

		ScriptInstance ins;
		CreateScriptInstance(*m_script, ins);
		for (auto& i : m_script->m_fields)
		{
			ScriptData& data = m_fields[i.first];
			data.type = i.second.field_type;
			if (i.second.field_type == ScriptFieldType::String) continue;
			ins.GetFieldValueInternal(i.first, data.data, 16);

		}

		DestroyScriptInstance(ins);

		return true;
	}

	void ScriptFieldInstance::Shutdown()
	{
	}

}