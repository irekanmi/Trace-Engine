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
		m_onCreate = other.m_onCreate;
		m_onStart = other.m_onStart;
		m_onUpdate = other.m_onUpdate;

	}

	ScriptInstance::~ScriptInstance()
	{
	}

	bool ScriptInstance::Init()
	{
		if (!m_script) return false;

		m_onCreate = m_script->GetMethod("OnCreate");
		m_onStart = m_script->GetMethod("OnStart");
		m_onUpdate = m_script->GetMethod("OnUpdate");

		for (auto& i : m_script->m_fields)
		{
			if (i.field_flags & ScriptFieldFlagBit::Public)
			{
				m_fields[i.field_name] = &i;
			}
		}

		return true;
	}

	void ScriptInstance::Shutdown()
	{
		m_fields.clear();

		m_onCreate = nullptr;
		m_onStart = nullptr;
		m_onUpdate = nullptr;
	}

	void ScriptInstance::OnCreate()
	{
		if (m_onCreate)
		{
			InvokeScriptMethod_Instance(*m_onCreate, *this, nullptr);
		}
	}

	void ScriptInstance::OnStart()
	{
		if (m_onStart)
		{
			InvokeScriptMethod_Instance(*m_onStart, *this, nullptr);
		}
	}

	void ScriptInstance::OnUpdate(float deltaTime)
	{
		if (m_onUpdate)
		{
			void* params[1] =
			{
				GetValueInternal(&deltaTime)
			};
			InvokeScriptMethod_Instance(*m_onUpdate, *this, params);
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size)
	{
		if (m_fields.find(field_name) != m_fields.end())
		{
			return GetInstanceFieldValue(*this, *m_fields.at(field_name), value, val_size);
		}

		return false;
	}

	void ScriptInstance::SetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size)
	{
		if (m_fields.find(field_name) != m_fields.end())
		{
			SetInstanceFieldValue(*this, *m_fields.at(field_name), value, val_size);
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

}