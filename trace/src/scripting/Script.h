#pragma once

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

	class ScriptInstance
	{

	public:
		ScriptInstance();
		ScriptInstance(const ScriptInstance& other);
		~ScriptInstance();

		bool Init();
		void Shutdown();

		void OnCreate();
		void OnStart();
		void OnUpdate(float deltaTime);

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

		void* m_internal = nullptr;
		Script* m_script = nullptr;
	private:
		bool GetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size);
		void SetFieldValueInternal(const std::string& field_name, void* value, uint32_t val_size);

	private:
		ScriptMethod* m_onCreate = nullptr;
		ScriptMethod* m_onStart = nullptr;
		ScriptMethod* m_onUpdate = nullptr;
		std::unordered_map<std::string, ScriptField*> m_fields;

	protected:


	};

	class Script
	{

	public:
		Script();
		~Script();

		ScriptInstance CreateInstance();
		ScriptMethod* GetMethod(const std::string& method);
		uintptr_t GetID();

		void* m_internal = nullptr;
		std::map<std::string, ScriptMethod> m_methods;
		std::vector<ScriptField> m_fields;
		std::string script_name;
	private:

	protected:

	};

}
