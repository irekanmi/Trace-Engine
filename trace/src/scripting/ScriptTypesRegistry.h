#pragma once

#include "reflection/TypeRegistry.h"
#include "scripting/Script.h"


namespace trace {

	REGISTER_TYPE(ScriptFieldType);
	REGISTER_TYPE(ScriptFieldFlagBit);
	REGISTER_TYPE(ScriptFieldFlag);

	BEGIN_REGISTER_CLASS(ScriptField)
		REGISTER_TYPE(ScriptField);
		REGISTER_MEMBER(ScriptField, field_name);
		REGISTER_MEMBER(ScriptField, field_type);
		REGISTER_MEMBER(ScriptField, field_flags);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ScriptData)
		REGISTER_TYPE(ScriptData);
		REGISTER_MEMBER(ScriptData, data);
		REGISTER_MEMBER(ScriptData, type);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ScriptFieldInstance)
		REGISTER_TYPE(ScriptFieldInstance);
		REGISTER_MEMBER(ScriptFieldInstance, m_fields);
	END_REGISTER_CLASS;


}
