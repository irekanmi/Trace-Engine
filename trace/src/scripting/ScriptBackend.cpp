#include "pch.h"


#include "ScriptBackend.h"
#include "core/FileSystem.h"
#include "core/io/Logging.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Componets.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>
#include "spdlog/fmt/fmt.h"


struct ComponentMap
{
	MonoClass* component_class = nullptr;
	std::unordered_map<UUID, MonoObject*> components_data;
};

struct MonoData
{
	MonoDomain* rootDomain = nullptr;
	MonoDomain* appDomain = nullptr;

	MonoAssembly* coreAssembly = nullptr;
	MonoImage* coreImage = nullptr;
	std::string mono_dir;


	Scene* scene = nullptr;
	MonoMethod* component_ctor = nullptr;
	std::unordered_map<MonoType*, ComponentMap> get_components;
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> has_components;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> remove_components;


};

struct MonoScript
{
	MonoClass* kClass;

};

struct MonoInstance
{
	MonoObject* kObject;
};

struct M_Method
{
	MonoMethod* kMethod;
};

MonoData s_MonoData;


static std::unordered_map<std::string, ScriptFieldType> s_FieldTypes = 
{
	{ "System.Single",Float},
	{ "System.String",String},
	{ "System.Int32",Int32},
	{ "System.UInt32",UInt32},
	{ "System.Int64",Int64},
	{ "System.UInt64",UInt64},
	{ "System.Double",Double},
	{ "System.Char",Char},
	{ "System.Int16",Int16},
	{ "System.UInt16",UInt16},
	{ "System.Boolean",Bool},
	{ "System.Byte",Byte},
	{ "Trace.Vec2",Vec2},
	{ "Trace.Vec3",Vec3},
	{ "Trace.Vec4",Vec4},
	{ "Trace.Action", Action}
};


char* ReadBytes(const std::string& filepath, uint32_t* outSize);
MonoAssembly* LoadAssembly(const std::string& filePath);
void PrintAssemblyTypes(MonoAssembly* assembly);
void LoadAssemblyTypes(MonoAssembly* assembly, std::unordered_map<std::string, Script>& data);

template<typename T>
bool RegisterComponent()
{
	std::string name = typeid(T).name();
	std::string comp_name = name.substr(name.find_last_of(':') + 1);

	std::string _name = fmt::format("Trace.{}", comp_name);
	MonoType* res = mono_reflection_type_from_name(_name.data(), s_MonoData.coreImage);
	if (!res)
	{
		TRC_WARN("{} is not present in the assembly", name);
		return false;
	}

	auto& get_comp = s_MonoData.get_components[res];
	get_comp.component_class = mono_class_from_name(s_MonoData.coreImage, "Trace", comp_name.c_str());;
	s_MonoData.has_components[res] = [](Entity& entity) -> bool { return entity.HasComponent<T>(); };
	s_MonoData.remove_components[res] = [](Entity& entity) { entity.RemoveComponent<T>(); };

	return true;
}

bool InitializeInternal(const std::string& bin_dir)
{


	mono_set_assemblies_path(bin_dir.c_str());

	
	s_MonoData.mono_dir = bin_dir;
	s_MonoData.rootDomain = mono_jit_init("TraceScritingEngine");
	s_MonoData.appDomain = mono_domain_create_appdomain("TraceAppDomain", nullptr);
	mono_domain_set(s_MonoData.appDomain, true);

	BindInternalFuncs();

	return true;
}
bool ShutdownInternal()
{
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(s_MonoData.appDomain);

	mono_jit_cleanup(s_MonoData.rootDomain);
	return true;
}


bool LoadComponents()
{

	bool result = true;

	result = RegisterComponent<TransformComponent>() && result;
	result = RegisterComponent<TagComponent>() && result;
	result = RegisterComponent<CameraComponent>() && result;
	result = RegisterComponent<LightComponent>() && result;
	result = RegisterComponent<MeshComponent>() && result;
	result = RegisterComponent<ModelComponent>() && result;
	result = RegisterComponent<ModelRendererComponent>() && result;
	result = RegisterComponent<TextComponent>() && result;
	result = RegisterComponent<RigidBodyComponent>() && result;
	result = RegisterComponent<BoxColliderComponent>() && result;
	result = RegisterComponent<SphereColliderComponent>() && result;

	return true;
}

bool LoadCoreAssembly(const std::string& file_path)
{
	s_MonoData.coreAssembly = LoadAssembly(file_path);
	s_MonoData.coreImage = mono_assembly_get_image(s_MonoData.coreAssembly);

	MonoClass* component_class = mono_class_from_name(s_MonoData.coreImage, "Trace", "Component");
	s_MonoData.component_ctor = mono_class_get_method_from_name(component_class, ".ctor", 1);

	PrintAssemblyTypes(s_MonoData.coreAssembly);

	return true;
}
bool LoadMainAssembly(const std::string& file_path)
{
	return true;
}

bool LoadAllScripts(std::unordered_map<std::string, Script>& out_data)
{

	LoadAssemblyTypes(s_MonoData.coreAssembly, out_data);

	return true;
}

bool OnSceneStartInternal(Scene* scene)
{
	s_MonoData.scene = scene;
	return true;
}

bool OnSceneStopInternal(Scene* scene)
{
	for (auto& i : s_MonoData.get_components)
	{
		i.second.components_data.clear();
	}

	s_MonoData.scene = nullptr;
	return true;
}

bool CreateScript(const std::string& name, Script& script, const std::string& nameSpace, bool core)
{

	if (!core)
	{
		//TODO: Implement for game assembly
		return true;
	}

	MonoClass* out_class = mono_class_from_name(s_MonoData.coreImage, nameSpace.c_str(), name.c_str());
	if (!out_class)
	{
		std::cout << "Failed to create script " << nameSpace << "." << name << std::endl;
		return false;
	}
	script.m_internal = out_class;
	if (nameSpace.empty())
		script.script_name = name;
	else script.script_name = nameSpace + '.' + name;


	uint32_t method_count = mono_class_num_methods(out_class);
	std::cout << nameSpace << "." << name << " has " << method_count << "methods" << std::endl;

	void* method_iter = nullptr;
	while (MonoMethod* method = mono_class_get_methods(out_class, &method_iter))
	{
		std::string method_name = mono_method_get_name(method);
		script.m_methods[method_name].m_internal = method;
	}

	uint32_t field_count = mono_class_num_fields(out_class);
	std::cout << nameSpace << "." << name << " has " << field_count << "fields" << std::endl;

	void* field_iter = nullptr;
	while (MonoClassField* field = mono_class_get_fields(out_class, &field_iter))
	{
		std::string field_name = mono_field_get_name(field);
		uint32_t flags = mono_field_get_flags(field);
		ScriptField field_res;
		field_res.field_name = field_name;
		if (flags & MONO_FIELD_ATTR_PUBLIC) field_res.field_flags |= ScriptFieldFlagBit::Public;
		if (flags & MONO_FIELD_ATTR_PRIVATE) field_res.field_flags |= ScriptFieldFlagBit::Private;
		std::string field_type = mono_type_get_name(mono_field_get_type(field));
		std::cout << field_name << " is of type " << field_type << std::endl;
		if (s_FieldTypes.find(field_type) != s_FieldTypes.end())
		{
			field_res.field_type = s_FieldTypes.at(field_type);
		}
		else field_res.field_type = ScriptFieldType::UnKnown;

		field_res.m_internal = field;

		script.m_fields[field_name] = field_res;
		
	}


	return true;
}
bool GetScriptID(Script& script, uintptr_t& res)
{
	if (!script.m_internal)
	{
		std::cout << "Script {" << script.script_name << "} is not valid" << std::endl;
		return false;
	}
	uint32_t token = mono_class_get_type_token((MonoClass*)script.m_internal);
	res = (uintptr_t)token;


	return true;
}
bool DestroyScript(Script& script)
{
	return true;
}
bool CreateScriptInstance(Script& script, ScriptInstance& out_instance)
{
	MonoObject* out_object = mono_object_new(s_MonoData.appDomain, (MonoClass*)script.m_internal);
	if (!out_object)
	{
		std::cout << "Failed to create script instance " << std::endl;
		return false;
	}
	mono_runtime_object_init(out_object);

	out_instance.m_internal = out_object;
	out_instance.m_script = &script;

	

	return true;
}
bool DestroyScriptInstance(ScriptInstance& instance)
{
	if (!instance.m_internal)
	{
		TRC_ERROR("Can't destory an invalid instance");
		return false;
	}

	
	instance.m_internal = nullptr;
	return true;
}

bool GetScriptMethod(const std::string& method_name, ScriptMethod& out_method, Script& script, int param_count)
{
	MonoMethod* _mthd = mono_class_get_method_from_name((MonoClass*)script.m_internal, method_name.c_str(), param_count);
	if (!_mthd)
	{
		std::cout << "Failed to GetMethod " << method_name << std::endl;
		return false;
	}
	out_method.m_internal = _mthd;

	return true;
}
bool InvokeScriptMethod_Instance(ScriptMethod& method, ScriptInstance& instance, void** params)
{
	MonoObject* exp = nullptr;
	mono_runtime_invoke((MonoMethod*)method.m_internal, (MonoObject*)instance.m_internal, params, &exp);
	if (exp) mono_print_unhandled_exception(exp);
	return true;
}
bool InvokeScriptMethod_Class(ScriptMethod& method, Script& script, void** params)
{
	return true;
}

bool GetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* out_value, uint32_t val_size)
{
	if (!instance.m_internal)
	{
		std::cout << "Invalid instance " << __FUNCTION__ << std::endl;

		return false;
	}

	mono_field_get_value((MonoObject*)instance.m_internal, (MonoClassField*)field.m_internal, out_value);

	return true;
}

bool SetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* value, uint32_t val_size)
{
	if (!instance.m_internal)
	{
		std::cout << "Invalid instance " << __FUNCTION__ << std::endl;

		return false;
	}

	mono_field_set_value((MonoObject*)instance.m_internal, (MonoClassField*)field.m_internal, value);

	return true;
}



MonoAssembly* LoadAssembly(const std::string& filePath)
{
	uint32_t file_size = 0;
	char* code = ReadBytes(filePath, &file_size);

	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_full(code, file_size, true, &status, false);

	if (status != MONO_IMAGE_OK)
	{
		TRC_ERROR("Failed to load image {}", filePath);
		return nullptr;
	}

	MonoAssembly* assembly = mono_assembly_load_from_full(image, filePath.c_str(), &status, false);
	mono_image_close(image);

	delete[] code;

	return assembly;
}

void PrintAssemblyTypes(MonoAssembly* assembly)
{
	MonoImage* image = mono_assembly_get_image(assembly);
	const MonoTableInfo* type_defs = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int32_t num_types = mono_table_info_get_rows(type_defs);

	for (int32_t i = 0; i < num_types; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(type_defs, i, cols, MONO_TYPEDEF_SIZE);

		const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

		std::cout << "Class : " << nameSpace << "." << name << std::endl;
	}
}

void LoadAssemblyTypes(MonoAssembly* assembly, std::unordered_map<std::string, Script>& data)
{
	MonoImage* image = mono_assembly_get_image(assembly);
	const MonoTableInfo* type_defs = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int32_t num_types = mono_table_info_get_rows(type_defs);

	for (int32_t i = 0; i < num_types; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(type_defs, i, cols, MONO_TYPEDEF_SIZE);

		const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		std::string fullName;
		if (nameSpace[0] != '\0')
		{
			fullName = std::string(nameSpace) + std::string(".") + std::string(name);
		}
		else
		{
			fullName = name;
		}

		CreateScript(name, data[fullName], nameSpace, true);
		
	}
}

char* ReadBytes(const std::string& filepath, uint32_t* outSize)
{
	

	FileHandle in_file;
	if (!FileSystem::open_file(filepath, (FileMode)(FileMode::READ | FileMode::BINARY), in_file))
	{
		TRC_ERROR("Failed to load file {}", filepath);
		return nullptr;
	}
	
	uint32_t size;
	FileSystem::read_all_bytes(in_file, nullptr, size);
	char* result = new char[size];
	*outSize = size;

	FileSystem::read_all_bytes(in_file, result, size);

	FileSystem::close_file(in_file);

	return result;

}

template<>
void* GetValueInternal<std::string>(std::string* value)
{
	return mono_string_new(s_MonoData.appDomain, value->c_str());
}


template<typename T>
void* GetValueInternal(T* value)
{
	static_assert(false);
	return nullptr;
}

template<>
void* GetValueInternal<float>(float* value)
{
	return value;
}

template<>
void* GetValueInternal<const char>(const char* value)
{
	return mono_string_new(s_MonoData.appDomain, value);
}


#pragma region Debug

void Debug_Log(MonoString* text)
{
	char* c_str = mono_string_to_utf8(text);
	std::string res(c_str);
	mono_free(c_str);
	TRC_TRACE(res);
}

void Debug_Info(MonoString* text)
{
	char* c_str = mono_string_to_utf8(text);
	std::string res(c_str);
	mono_free(c_str);
	TRC_INFO(res);
}

void Debug_Trace(MonoString* text)
{
	char* c_str = mono_string_to_utf8(text);
	std::string res(c_str);
	mono_free(c_str);
	TRC_TRACE(res);
}

#pragma endregion


#pragma region Action

MonoObject* Action_GetComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	auto it = s_MonoData.get_components.find(type);
	
	ComponentMap& comps = it->second;
	auto obj_it = comps.components_data.find(uuid);
	if (obj_it != comps.components_data.end())
	{
		return obj_it->second;
	}
	else
	{
		UUID val = uuid;
		MonoObject* res = mono_object_new(s_MonoData.appDomain, comps.component_class);
		comps.components_data[uuid] = res;
		mono_runtime_object_init(res);
		MonoObject* exp = nullptr;
		void* params[1] =
		{
			&val
		};
		mono_runtime_invoke((MonoMethod*)s_MonoData.component_ctor, res, params, &exp);
		if (exp) mono_print_unhandled_exception(exp);
		return res;
	}

}

MonoObject* Action_GetScript(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	uint32_t token = mono_class_get_type_token(mono_type_get_class(type));

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	MonoObject* res = (MonoObject*)entity.GetScript((uintptr_t)token)->m_internal;

	return res;

}

bool Action_HasComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return false;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	auto it = s_MonoData.has_components.find(type);

	if (it == s_MonoData.has_components.end())
	{
		TRC_WARN("Component is not valid, => {}", mono_type_get_name(type));
		return false;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return false;
	}

	return it->second(entity);

}

bool Action_HasScript(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	uint32_t token = mono_class_get_type_token(mono_type_get_class(type));

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return false;
	}

	return entity.HasScript((uintptr_t)token);
}

void Action_RemoveComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	auto it = s_MonoData.remove_components.find(type);

	if (it == s_MonoData.remove_components.end())
	{
		TRC_WARN("Component is not valid, => {}", mono_type_get_name(type));
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	it->second(entity);
	return;

}

void Action_RemoveScript(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	uint32_t token = mono_class_get_type_token(mono_type_get_class(type));

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	entity.RemoveScript((uintptr_t)token);
}

#pragma endregion

#pragma region TransformComponent

void TransformComponent_GetPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	glm::vec3 res = entity.GetComponent<TransformComponent>()._transform.GetPosition();
	*position = res;
}

void TransformComponent_SetPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid");
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	entity.GetComponent<TransformComponent>()._transform.SetPosition(*position);
}

#pragma endregion


#define ADD_INTERNAL_CALL(func) mono_add_internal_call("Trace.InternalCalls::"#func, &func)

void BindInternalFuncs()
{
	ADD_INTERNAL_CALL(Debug_Info);
	ADD_INTERNAL_CALL(Debug_Log);
	ADD_INTERNAL_CALL(Debug_Trace);

	ADD_INTERNAL_CALL(Action_GetComponent);
	ADD_INTERNAL_CALL(Action_GetScript);
	ADD_INTERNAL_CALL(Action_HasComponent);
	ADD_INTERNAL_CALL(Action_HasScript);
	ADD_INTERNAL_CALL(Action_RemoveComponent);
	ADD_INTERNAL_CALL(Action_RemoveScript);


	ADD_INTERNAL_CALL(TransformComponent_GetPosition);
	ADD_INTERNAL_CALL(TransformComponent_SetPosition);


}