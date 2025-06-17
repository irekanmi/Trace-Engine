#pragma once


#include "reflection/TypeRegistry.h"
#include "core/Coretypes.h"

#include <string>
#include <map>
#include <unordered_map>
#include <vector>

namespace trace {

	class Script;

	enum ScriptFieldType
	{
		UnKnown,
		Float,
		String,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Double,
		Char,
		Int16,
		UInt16,
		Bool,
		Byte,
		Vec2,
		Vec3,
		Vec4,
		Action
	};

	enum ScriptFieldFlagBit
	{
		Public = 1 << 1,
		Private = 1 << 2,
	};

	using ScriptFieldFlag = uint32_t;

	class ScriptField
	{

	public:
		std::string field_name;
		ScriptFieldType field_type;
		ScriptFieldFlag field_flags = 0;
		void* m_internal = nullptr;
	private:
	protected:

	};

	class ScriptMethod
	{

	public:
		ScriptMethod();
		~ScriptMethod();

		void* m_internal = nullptr;
	private:

	protected:

	};

	struct ScriptData
	{
		char data[16];
		void* ptr = nullptr;
		ScriptFieldType type;
	};

	class ScriptFieldInstance
	{

	public:
		ScriptFieldInstance();
		ScriptFieldInstance(const ScriptFieldInstance& other) = default;
		~ScriptFieldInstance();

		bool Init(Script* script);
		void Shutdown();
		bool Reload();

		//NOTE: To be used for builtin types not for (std types and custom types)
		template<typename T>
		bool GetValue(const std::string& field_name, T& out_value)
		{
			static_assert(sizeof(T) <= 16);
			auto it = m_fields.find(field_name);
			if (it == m_fields.end()) return false;
			out_value = *(T*)&it->second.data;
			return true;
		}

		//NOTE: To be used for builtin types not for (std types and custom types)
		template<typename T>
		void SetValue(const std::string& field_name, T& value)
		{
			static_assert(sizeof(T) <= 16);
			auto it = m_fields.find(field_name);
			if (it == m_fields.end()) return;
			memcpy_s(it->second.data, 16, &value, sizeof(T));
		}

		
		std::unordered_map<std::string, ScriptData>& GetFields() { return m_fields; }
		Script* GetScript() { return m_script; }

		void GetFields(std::unordered_map<std::string, ScriptData>& fields_data) { m_fields = fields_data; }
		void SetScript(Script* script) { m_script = script; }

	private:
		std::unordered_map<std::string, ScriptData> m_fields;
		Script* m_script = nullptr;
		
	private:
		ACCESS_CLASS_MEMBERS(ScriptFieldInstance);

	protected:


	};

	class ScriptInstance
	{

	public:
		ScriptInstance();
		ScriptInstance(const ScriptInstance& other);
		~ScriptInstance();

		bool Init();
		void Shutdown();
		std::unordered_map<std::string, ScriptField*>& GetFields() { return m_fields; }


		//NOTE: To be used for builtin types not for (std types and custom types)
		template<typename T>
		bool GetFieldValue(const std::string& field_name, T& out_value)
		{
			return GetFieldValueInternal(field_name, &out_value, sizeof(T));
		}

		//NOTE: To be used for builtin types not for (std types and custom types)
		template<typename T>
		void SetFieldValue(const std::string& field_name, T& value)
		{
			SetFieldValueInternal(field_name, &value, sizeof(T));
		}

		void* GetBackendHandle();

		Script* GetScript() { return m_script; }
		void* GetInternal() { return m_internal; }

		void SetScript(Script* script) { m_script = script; }
		void SetInternal(void* internal_data) { m_internal = internal_data; }

	public:
		bool GetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size);
		void SetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size);

	private:
		void* m_internal = nullptr;
		Script* m_script = nullptr;
		std::unordered_map<std::string, ScriptField*> m_fields;

	protected:
		friend class ScriptFieldInstance;
		friend class ScriptRegistry;
		friend class Scene;


	};

	class Script
	{

	public:
		Script();
		~Script();

		ScriptInstance CreateInstance();
		ScriptMethod* GetMethod(const std::string& method);
		ScriptMethod* GetMethod(StringID method);
		uintptr_t GetID();

		void* GetInternal() { return m_internal; }
		std::unordered_map<StringID, ScriptMethod>& GetMethods() { return m_methods; }
		std::unordered_map<std::string, ScriptField>& GetFields() { return m_fields; }
		std::string& GetScriptName() { return m_scriptName; }

		void SetInternal(void* internal_data) { m_internal = internal_data; }
		void SetMethods(std::unordered_map<StringID, ScriptMethod>& methods) { m_methods = methods; }
		void SetFields(std::unordered_map<std::string, ScriptField>& fields) { m_fields = fields; }
		void SetScriptName(const std::string& name) { m_scriptName = name; }


	private:
		void* m_internal = nullptr;
		std::unordered_map<StringID, ScriptMethod> m_methods;
		std::unordered_map<std::string, ScriptField> m_fields;
		std::string m_scriptName;

	protected:

	};

}
