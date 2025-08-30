#include "pch.h"


#include "ScriptBackend.h"
#include "core/FileSystem.h"
#include "core/io/Logging.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "core/input/Input.h"
#include "scripting/ScriptEngine.h"
#include "backends/Physicsutils.h"
#include "external_utils.h"
#include "core/Utils.h"
#include "networking/NetworkTypes.h"
#include "networking/NetworkManager.h"
#include "serialize/DataStream.h"
#include "debug/Debugger.h"
#include "resource/GenericAssetManager.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/threads.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>
#include "spdlog/fmt/fmt.h"


struct ComponentMap
{
	MonoClass* component_class = nullptr;
	//std::unordered_map<UUID, MonoObject*> components_data;
};

struct MonoData
{
	MonoDomain* rootDomain = nullptr;
	MonoDomain* appDomain = nullptr;

	MonoAssembly* coreAssembly = nullptr;
	MonoImage* coreImage = nullptr;
	std::string mono_dir;

	MonoAssembly* mainAssembly = nullptr;
	MonoImage* mainImage = nullptr;

	bool reload_assembly = false;//NOTE: This variable is needed so that threads can release their attachment to the app domain
	Scene* scene = nullptr;
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> has_components;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> remove_components;
	std::unordered_map<MonoType*, std::function<void(Entity&, ScriptMethod*, ScriptInstance*)>> iterate_components;
	std::unordered_map<MonoType*, std::function<Entity()>> get_entity_with_component;

	MonoClass* physics_class = nullptr;
	MonoMethod* on_collision_enter = nullptr;
	MonoMethod* on_collision_exit = nullptr;
	MonoMethod* on_trigger_enter = nullptr;
	MonoMethod* on_trigger_exit = nullptr;

};

struct MonoScript
{
	MonoClass* kClass;

};

struct MonoInstance
{
	MonoObject* kObject;
	uint32_t kHandle = 0;
};

struct M_Method
{
	MonoMethod* kMethod;
};

MonoData s_MonoData;


static std::unordered_map<std::string, ScriptFieldType> s_FieldTypes = 
{
	{ "System.Single",ScriptFieldType::Float},
	{ "System.String",ScriptFieldType::String},
	{ "System.Int32",ScriptFieldType::Int32},
	{ "System.UInt32",ScriptFieldType::UInt32},
	{ "System.Int64",ScriptFieldType::Int64},
	{ "System.UInt64",ScriptFieldType::UInt64},
	{ "System.Double",ScriptFieldType::Double},
	{ "System.Char",ScriptFieldType::Char},
	{ "System.Int16",ScriptFieldType::Int16},
	{ "System.UInt16",ScriptFieldType::UInt16},
	{ "System.Boolean",ScriptFieldType::Bool},
	{ "System.Byte",ScriptFieldType::Byte},
	{ "Trace.Vec2",ScriptFieldType::Vec2},
	{ "Trace.Vec3",ScriptFieldType::Vec3},
	{ "Trace.Vec4",ScriptFieldType::Vec4},
	{ "Trace.Action", ScriptFieldType::Action},
	{ "Trace.Prefab", ScriptFieldType::Prefab}
};


char* ReadBytes(const std::string& filepath, uint32_t* outSize);
MonoAssembly* LoadAssembly(const std::string& filePath);
void PrintAssemblyTypes(MonoAssembly* assembly);
void LoadAssemblyTypes(MonoAssembly* assembly, std::unordered_map<std::string, Script>& data, bool core);



template<typename... Component>
void RegisterComponent()
{

	([&]() 
		{
			std::string name = typeid(Component).name();
			std::string comp_name = name.substr(name.find_last_of(':') + 1);

			std::string _name = fmt::format("Trace.{}", comp_name);
			MonoType* res = mono_reflection_type_from_name(_name.data(), s_MonoData.coreImage);
			if (!res)
			{
				TRC_WARN("{} is not present in the assembly", name);
				return;
			}

			/*auto& get_comp = s_MonoData.get_components[res];
			get_comp.component_class = mono_class_from_name(s_MonoData.coreImage, "Trace", comp_name.c_str());;*/
			s_MonoData.has_components[res] = [](Entity& entity) -> bool { return entity.HasComponent<Component>(); };
			s_MonoData.remove_components[res] = [](Entity& entity) { entity.RemoveComponent<Component>(); };
			s_MonoData.iterate_components[res] = [](Entity& entity, ScriptMethod* method, ScriptInstance* src_instance)
			{ 
				Scene* scene = entity.GetScene();
				scene->IterateComponent<Component>([method, src_instance](Entity obj) -> bool
					{
						UUID id = obj.GetID();
						MonoObject* ins = (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(id)->GetBackendHandle();

						void* params[] =
						{
							ins,
							&id
						};


						InvokeScriptMethod_Instance(*method, *src_instance, params);

						return false;
					});
			};
			s_MonoData.get_entity_with_component[res] = []() -> Entity
			{ 
				Scene* scene = s_MonoData.scene;
				Entity result;
				if (!scene)
				{
					return result;
				}
				scene->IterateComponent<Component>([&result](Entity obj) -> bool
					{
						result = obj;
						return true;
					});

				return result;
			};
		}(), ...);


}

template<typename... Component>
void RegisterComponent(ComponentGroup<Component...>)
{
	RegisterComponent<Component...>();
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

	RegisterComponent(AllComponents{});

	return true;
}

bool LoadCoreAssembly(const std::string& file_path)
{
	s_MonoData.coreAssembly = LoadAssembly(file_path);
	s_MonoData.coreImage = mono_assembly_get_image(s_MonoData.coreAssembly);

	MonoClass* physics_class = mono_class_from_name(s_MonoData.coreImage, "Trace", "Physics");
	TRC_ASSERT(physics_class != nullptr, "Can't find Physics class inside assembly, Function: {}", __FUNCTION__);
	s_MonoData.on_collision_enter = mono_class_get_method_from_name(physics_class, "OnCollisionEnter", 2);
	s_MonoData.on_collision_exit = mono_class_get_method_from_name(physics_class, "OnCollisionExit", 2);
	s_MonoData.on_trigger_enter = mono_class_get_method_from_name(physics_class, "OnTriggerEnter", 2);
	s_MonoData.on_trigger_exit = mono_class_get_method_from_name(physics_class, "OnTriggerExit", 2);
	TRC_ASSERT(s_MonoData.on_collision_enter != nullptr, "Can't find OnCollisionEnter inside Physics class, Function: {}", __FUNCTION__);
	TRC_ASSERT(s_MonoData.on_collision_exit != nullptr, "Can't find OnCollisionExit inside Physics class, Function: {}", __FUNCTION__);
	TRC_ASSERT(s_MonoData.on_trigger_enter != nullptr, "Can't find OnTriggerEnter inside Physics class, Function: {}", __FUNCTION__);
	TRC_ASSERT(s_MonoData.on_trigger_exit != nullptr, "Can't find OnTriggerExit inside Physics class, Function: {}", __FUNCTION__);
	s_MonoData.physics_class = physics_class;

	PrintAssemblyTypes(s_MonoData.coreAssembly);

	return true;
}
bool LoadMainAssembly(const std::string& file_path)
{
	if (s_MonoData.mainAssembly)
	{
		mono_assembly_close(s_MonoData.mainAssembly);
		s_MonoData.mainAssembly = nullptr;
	}

	s_MonoData.mainAssembly = LoadAssembly(file_path);
	s_MonoData.mainImage = mono_assembly_get_image(s_MonoData.mainAssembly);

	PrintAssemblyTypes(s_MonoData.mainAssembly);

	return true;
}

bool LoadAllScripts(std::unordered_map<std::string, Script>& out_data)
{

	LoadAssemblyTypes(s_MonoData.coreAssembly, out_data, true);
	if (s_MonoData.mainAssembly) LoadAssemblyTypes(s_MonoData.mainAssembly, out_data, false);

	return true;
}

bool ReloadAssemblies(const std::string& core_assembly, const std::string& main_assembly)
{
	MonoDomain* app_domain = s_MonoData.appDomain;
	s_MonoData.appDomain = nullptr;
	s_MonoData.reload_assembly = true;
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(app_domain);

	s_MonoData.appDomain = mono_domain_create_appdomain("TraceAppDomain", nullptr);
	mono_domain_set(s_MonoData.appDomain, true);

	LoadCoreAssembly(core_assembly);


	s_MonoData.mainAssembly = LoadAssembly(main_assembly);
	s_MonoData.mainImage = mono_assembly_get_image(s_MonoData.mainAssembly);

	PrintAssemblyTypes(s_MonoData.mainAssembly);

	s_MonoData.reload_assembly = false;

	return true;
}

bool OnSceneStartInternal(Scene* scene)
{
	s_MonoData.scene = scene;
	return true;
}

bool OnSceneStopInternal(Scene* scene)
{
	/*for (auto& i : s_MonoData.get_components)
	{
		i.second.components_data.clear();
	}*/

	s_MonoData.scene = nullptr;
	return true;
}

bool CreateScript(const std::string& name, Script& script, const std::string& nameSpace, bool core)
{

	MonoImage* image = core ? s_MonoData.coreImage : s_MonoData.mainImage;
	MonoClass* out_class = mono_class_from_name(image, nameSpace.c_str(), name.c_str());
	if (!out_class)
	{
		TRC_ERROR("Failed to create script {}.{}",nameSpace , name );
		return false;
	}
	script.SetInternal(out_class);
	if (nameSpace.empty())
		script.SetScriptName(name);
	else script.SetScriptName(nameSpace + '.' + name);


	uint32_t method_count = mono_class_num_methods(out_class);
	std::cout << nameSpace << "." << name << " has " << method_count << "methods" << std::endl;

	void* method_iter = nullptr;
	while (MonoMethod* method = mono_class_get_methods(out_class, &method_iter))
	{
		std::string method_name = mono_method_get_name(method);		
		script.GetMethods()[STR_ID(method_name)].m_internal = method;
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
		if ((flags & MONO_FIELD_ATTR_PUBLIC) == MONO_FIELD_ATTR_PUBLIC)
		{
			field_res.field_flags |= ScriptFieldFlagBit::Public;
		}
		if ((flags & MONO_FIELD_ATTR_PRIVATE) == MONO_FIELD_ATTR_PRIVATE)
		{
			field_res.field_flags |= ScriptFieldFlagBit::Private;
		}
		std::string field_type = mono_type_get_name(mono_field_get_type(field));
		TRC_INFO("{} is of type {}", field_name, field_type);
		if (s_FieldTypes.find(field_type) != s_FieldTypes.end())
		{
			field_res.field_type = s_FieldTypes.at(field_type);
		}
		else
		{
			field_res.field_type = ScriptFieldType::UnKnown;
		}

		field_res.m_internal = field;

		script.GetFields()[field_name] = field_res;
		
	}


	return true;
}
bool GetScriptID(Script& script, uintptr_t& res)
{
	if (!script.GetInternal())
	{
		std::cout << "Script {" << script.GetScriptName() << "} is not valid" << std::endl;
		return false;
	}
	res = (uintptr_t)mono_class_get_type((MonoClass*)script.GetInternal());


	return true;
}
bool DestroyScript(Script& script)
{
	script.GetFields().clear();
	script.SetScriptName("");
	script.GetMethods().clear();
	script.SetInternal(nullptr);
	return true;
}

bool CreateScriptInstance(Script& script, ScriptInstance& out_instance)
{
	MonoObject* out_object = mono_object_new(s_MonoData.appDomain, (MonoClass*)script.GetInternal());
	if (!out_object)
	{
		TRC_ERROR("Failed to create script instance, Funtion: {} ", __FUNCTION__);
		return false;
	}
	mono_runtime_object_init(out_object);

	MonoInstance* ins = new MonoInstance; //TODO: Use custom allocator
	ins->kObject = out_object;
	ins->kHandle = mono_gchandle_new(out_object, TRUE);
	out_instance.SetInternal(ins);
	out_instance.SetScript(&script);

	

	return true;
}
bool CreateScriptInstanceNoInit(Script& script, ScriptInstance& out_instance)
{
	MonoObject* out_object = mono_object_new(s_MonoData.appDomain, (MonoClass*)script.GetInternal());
	if (!out_object)
	{
		std::cout << "Failed to create script instance " << std::endl;
		return false;
	}

	MonoInstance* ins = new MonoInstance; //TODO: Use custom allocator
	ins->kObject = out_object;
	ins->kHandle = mono_gchandle_new(out_object, TRUE);
	out_instance.SetInternal(ins);
	out_instance.SetScript(&script);



	return true;
}
bool DestroyScriptInstance(ScriptInstance& instance)
{
	if (!instance.GetInternal())
	{
		TRC_ERROR("Can't destory an invalid instance");
		return false;
	}

	MonoInstance* ins = (MonoInstance*)instance.GetInternal();

	mono_gchandle_free(ins->kHandle);

	delete ins;//TODO: Use custom allocator

	instance.SetInternal(nullptr);
	return true;
}

bool GetScriptInstanceHandle(ScriptInstance& instance, void*& out)
{
	if (!instance.GetInternal()) return false;

	MonoInstance* ins = (MonoInstance*)instance.GetInternal();

	out = ins->kObject;
	return true;
}

bool CloneScriptInstance(ScriptInstance* src, ScriptInstance* dst)
{
	MonoObject* out_object = mono_object_clone((MonoObject*)src->GetBackendHandle());
	if (!out_object)
	{
		TRC_ERROR("Failed to create script instance, Funtion: {} ", __FUNCTION__);
		return false;
	}
	mono_runtime_object_init(out_object);

	MonoInstance* ins = new MonoInstance; //TODO: Use custom allocator
	ins->kObject = out_object;
	ins->kHandle = mono_gchandle_new(out_object, TRUE);
	dst->SetInternal(ins);

	return true;
}


bool GetScriptMethod(const std::string& method_name, ScriptMethod& out_method, Script& script, int param_count)
{
	MonoMethod* _mthd = mono_class_get_method_from_name((MonoClass*)script.GetInternal(), method_name.c_str(), param_count);
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
	MonoObject* obj = nullptr;
	GetScriptInstanceHandle(instance, (void*&)obj);
	mono_runtime_invoke((MonoMethod*)method.m_internal, obj, params, &exp);
	if (exp) mono_print_unhandled_exception(exp);
	return true;
}
bool InvokeScriptMethod_Class(ScriptMethod& method, Script& script, void** params)
{
	MonoObject* exp = nullptr;
	MonoObject* obj = nullptr;
	mono_runtime_invoke((MonoMethod*)method.m_internal, obj, params, &exp);
	if (exp) mono_print_unhandled_exception(exp);
	return true;
}

bool GetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* out_value, uint32_t val_size)
{
	if (!instance.GetInternal())
	{
		return false;
	}

	MonoObject* obj = nullptr;
	GetScriptInstanceHandle(instance, (void*&)obj);

	if (field.field_type == ScriptFieldType::Action)
	{
		MonoObject* result = nullptr;
		mono_field_get_value(obj, (MonoClassField*)field.m_internal, &result);

		if (!result)
		{
			return false;
		}

		ScriptField& Id_field = ScriptEngine::get_instance()->GetIDField();

		UUID Id = 0;
		mono_field_get_value(result, (MonoClassField*)Id_field.m_internal, &Id);
		
		if (Id != 0)
		{
			memcpy(out_value, &Id, sizeof(UUID));
		}

	}
	else
	{
		mono_field_get_value(obj, (MonoClassField*)field.m_internal, out_value);
	}

	return true;
}

bool SetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* value, uint32_t val_size)
{
	if (!instance.GetInternal())
	{
		std::cout << "Invalid instance " << __FUNCTION__ << std::endl;

		return false;
	}

	MonoObject* obj = nullptr;
	GetScriptInstanceHandle(instance, (void*&)obj);

	if (field.field_type == ScriptFieldType::Action)
	{
		UUID Id = 0;
		memcpy(&Id, value, sizeof(UUID));
		if (!s_MonoData.scene->GetEntity(Id))
		{
			TRC_WARN("Entity with ID: {}, is not part of the scene, Function: {}", Id, __FUNCTION__);
			Id = 0;
		}
		MonoObject* result = (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(Id)->GetBackendHandle();
		mono_field_set_value(obj, (MonoClassField*)field.m_internal, result);
		
	}
	else
	{
		mono_field_set_value(obj, (MonoClassField*)field.m_internal, value);
	}

	return true;
}

void AttachThread(void*& thread_info)
{
	if (s_MonoData.reload_assembly)
	{
		if (thread_info)
		{
			DetachThread(thread_info);
			thread_info = nullptr;
		}

	}
	else if (!thread_info && s_MonoData.appDomain)
	{
		thread_info = mono_thread_attach(s_MonoData.appDomain);
	}
}

void DetachThread(void* thread_info)
{
	if (thread_info == nullptr)
	{
		return;
	}
	mono_thread_detach((MonoThread*)thread_info);
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

void LoadAssemblyTypes(MonoAssembly* assembly, std::unordered_map<std::string, Script>& data, bool core)
{
	MonoImage* image = mono_assembly_get_image(assembly);
	const MonoTableInfo* type_defs = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int32_t num_types = mono_table_info_get_rows(type_defs);

	MonoClass* action = mono_class_from_name(s_MonoData.coreImage, "Trace", "Action");

	for (int32_t i = 0; i < num_types; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(type_defs, i, cols, MONO_TYPEDEF_SIZE);

		const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		std::string full_name;
		if (nameSpace[0] != '\0')
		{
			full_name = std::string(nameSpace) + std::string(".") + std::string(name);
		}
		else
		{
			full_name = name;
		}

		MonoClass* k_class =  mono_class_from_name(image, nameSpace, name);
		if (!k_class)
		{
			TRC_ERROR("Couldn't find a class, Name: {}", full_name);
			continue;
		}
		bool is_action = (action != k_class) && mono_class_is_subclass_of(k_class, action, false);
		

		if(is_action) CreateScript(name, data[full_name], nameSpace, core);
		
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
	char* result = new char[size];//TODO: Use custom allocator
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

void Debug_Sphere(glm::vec3* position, float radius, uint32_t steps, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), *position);
	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->DrawDebugSphere(radius, steps, transform, final_color);

}

#pragma endregion


#pragma region Action

MonoObject* Action_GetComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}
	return nullptr;
}

MonoObject* Action_GetScript(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	MonoObject* res = (MonoObject*)entity.GetScript((uintptr_t)type)->GetBackendHandle();

	return res;

}

MonoObject* Action_AddScript(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	MonoObject* res = (MonoObject*)entity.AddScript((uintptr_t)type)->GetBackendHandle();

	return res;

}

bool Action_HasComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
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
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return false;
	}

	return entity.HasScript((uintptr_t)type);
}

void Action_RemoveComponent(UUID uuid, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
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
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	entity.RemoveScript((uintptr_t)type);
}

MonoString* Action_GetName(UUID uuid)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return nullptr;
	}

	std::string& name = entity.GetComponent<TagComponent>().GetTag();

	return mono_string_new(s_MonoData.appDomain, name.c_str());

}

bool Action_IsOwner(UUID uuid)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return true;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return true;
	}


	return entity.IsOwner();

}

uint32_t Action_GetNetID(UUID uuid)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return 0;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return 0;
	}


	return entity.GetNetID();

}

#pragma endregion

#pragma region TransformComponent

void TransformComponent_GetPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
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
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	entity.GetComponent<TransformComponent>()._transform.SetPosition(*position);

	//Update Physics objects internal state
	if (CharacterControllerComponent* controller = entity.TryGetComponent<CharacterControllerComponent>())
	{
		PhysicsFunc::SetCharacterControllerPosition(controller->character, *position);
	}

}

void TransformComponent_GetWorldPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	*position = s_MonoData.scene->GetEntityWorldPosition(entity);
}

void TransformComponent_SetWorldPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	s_MonoData.scene->SetEntityWorldPosition(entity, *position);
}

void TransformComponent_GetWorldRotation(UUID id, glm::quat* rotation)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	*rotation = s_MonoData.scene->GetEntityWorldRotation(entity);
}

void TransformComponent_SetWorldRotation(UUID id, glm::quat* rotation)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	s_MonoData.scene->SetEntityWorldRotation(entity, *rotation);
}

void TransformComponent_GetRotation(UUID id, glm::quat* rotation)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	glm::quat res = entity.GetComponent<TransformComponent>()._transform.GetRotation();
	*rotation = res;
}

void TransformComponent_SetRotation(UUID id, glm::quat* rotation)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	entity.GetComponent<TransformComponent>()._transform.SetRotation(*rotation);
}

void TransformComponent_Forward(UUID id, glm::vec3* forward)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	*forward = entity.GetComponent<TransformComponent>()._transform.GetForward();
}

void TransformComponent_Right(UUID id, glm::vec3* right)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	*right = entity.GetComponent<TransformComponent>()._transform.GetRight();
}

void TransformComponent_Up(UUID id, glm::vec3* up)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	*up = entity.GetComponent<TransformComponent>()._transform.GetUp();
}

#pragma endregion

#pragma region Input

bool Input_GetKey(Keys key)
{
	return InputSystem::get_instance()->GetKey(key);
}

bool Input_GetKeyPressed(Keys key)
{
	return InputSystem::get_instance()->GetKeyPressed(key);
}

bool Input_GetKeyReleased(Keys key)
{
	return InputSystem::get_instance()->GetKeyReleased(key);
}

bool Input_GetButton(Buttons button)
{
	return InputSystem::get_instance()->GetButton(button);
}

bool Input_GetButtonPressed(Buttons button)
{
	return InputSystem::get_instance()->GetButtonPressed(button);
}

bool Input_GetButtonReleased(Buttons button)
{
	return InputSystem::get_instance()->GetButtonReleased(button);
}

#pragma endregion

#pragma region TextComponent

MonoString* TextComponent_GetString(UUID id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return nullptr;
	}

	std::string& txt = entity.GetComponent<TextComponent>().text;

	if (txt.empty()) return nullptr;

	return mono_string_new(s_MonoData.appDomain, txt.c_str());

}

void TextComponent_SetString(UUID id, MonoString* string)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	std::string& txt = entity.GetComponent<TextComponent>().text;

	char* c_str = mono_string_to_utf8(string);
	txt = c_str;
	mono_free(c_str);

}

#pragma endregion

#pragma region Scene

MonoObject* Scene_GetEntityByName(uint64_t string_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	StringID s_id;
	s_id.value = string_id;
	Entity entity = s_MonoData.scene->GetEntityByName(s_id);
	if (!entity)
	{
		TRC_ERROR("Entity is not present in scene, Scene Name: {}, Function: {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(entity.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_GetEntity(uint64_t entity_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}


	Entity entity = s_MonoData.scene->GetEntity(entity_id);
	if (!entity)
	{
		TRC_ERROR("Entity is not present in the current scene. Scene Name: {}, Function: {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(entity.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_GetChildEntityByName(UUID id, uint64_t string_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	StringID s_id;
	s_id.value = string_id;
	Entity parent = s_MonoData.scene->GetEntity(id);
	if (!parent)
	{
		TRC_ERROR("Entity is not present in scene, Scene Name: {}, Function: {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	Entity entity = s_MonoData.scene->GetChildEntityByName(parent, s_id);
	if (!entity)
	{
		TRC_ERROR("Can't find child. Scene Name: {}", s_MonoData.scene->GetName());
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(entity.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_InstanciateEntity_Position(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}


	Entity entity = s_MonoData.scene->GetEntity(id);
	if (!entity)
	{
		TRC_ERROR("Entity is presented in scene. Scene Name: {}, Function, {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	Entity result = s_MonoData.scene->InstanciateEntity(entity, *position);
	TRC_ASSERT(result, "Unable to Instaciate Entity, Funciton: {}", __FUNCTION__);
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_InstanciateEntity_Prefab_Position(UUID prefab_id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}


	Ref<Prefab> prefab = GenericAssetManager::get_instance()->Get<Prefab>(prefab_id);
	if (!prefab)
	{
		TRC_ERROR("Prefab not found. Prefab handle: {}, Function, {}", prefab_id, __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	Entity result = s_MonoData.scene->InstanciatePrefab(prefab);
	TRC_ASSERT(result, "Unable to Instaciate Prefab, Funciton: {}", __FUNCTION__);
	result.GetComponent<TransformComponent>()._transform.SetPosition(*position);
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_InstanciateEntity_Prefab_Position_NetID(UUID prefab_id, glm::vec3* position, uint32_t owner_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}


	Ref<Prefab> prefab = GenericAssetManager::get_instance()->Get<Prefab>(prefab_id);
	if (!prefab)
	{
		TRC_ERROR("Prefab not found. Prefab handle: {}, Function, {}", prefab_id, __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	Entity result = s_MonoData.scene->InstanciatePrefab(prefab, owner_id);
	TRC_ASSERT(result, "Unable to Instaciate Prefab, Funciton: {}", __FUNCTION__);
	result.GetComponent<TransformComponent>()._transform.SetPosition(*position);
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

MonoObject* Scene_InstanciateEntity_Position_NetID(UUID id, glm::vec3* position, uint32_t owner_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}


	Entity entity = s_MonoData.scene->GetEntity(id);
	if (!entity)
	{
		TRC_ERROR("Entity is presented in scene. Scene Name: {}, Function, {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	uint32_t instance_id = net_manager->GetInstanceID();

	Entity result = s_MonoData.scene->InstanciateEntity(entity, *position, owner_id);
	TRC_ASSERT(result, "Unable to Instaciate Entity, Funciton: {}", __FUNCTION__);
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());

	return (MonoObject*)ins->GetBackendHandle();
}

void Scene_IterateComponent(UUID id, MonoObject* src, uint64_t func_name_id, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	auto it = s_MonoData.iterate_components.find(type);

	if (it == s_MonoData.iterate_components.end())
	{
		TRC_WARN("Component is not valid, => {}", mono_type_get_name(type));
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	MonoType* class_type = mono_class_get_type(mono_object_get_class(src));
	ScriptInstance* instance = entity.GetScript((uintptr_t)class_type);
	if (!instance)
	{
		TRC_ASSERT(false, "These is not suppose to happen");
	}

	trace::StringID string_id;
	string_id.value = func_name_id;
	ScriptMethod* method = instance->GetScript()->GetMethod(string_id);
	if (!method)
	{
		TRC_ASSERT(false, "These is not suppose to happen");
	}

	it->second(entity, method, instance);
	
}

void Scene_IterateEntityScripts(UUID entity_id, UUID id, MonoObject* src, uint64_t func_name_id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}


	Entity src_entity = s_MonoData.scene->GetEntity(id);

	MonoType* class_type = mono_class_get_type(mono_object_get_class(src));
	ScriptInstance* instance = src_entity.GetScript((uintptr_t)class_type);
	if (!instance)
	{
		TRC_ASSERT(false, "These is not suppose to happen");
	}

	trace::StringID string_id;
	string_id.value = func_name_id;
	ScriptMethod* method = instance->GetScript()->GetMethod(string_id);
	if (!method)
	{
		TRC_ASSERT(false, "These is not suppose to happen");
	}

	s_MonoData.scene->GetScriptRegistry().Iterate(entity_id, [method, instance](UUID, Script*, ScriptInstance* script_instance)
		{
			MonoObject* obj = (MonoObject*)script_instance->GetBackendHandle();
			TRC_ASSERT(obj, "This is not supposed to happen");

			void* params[] =
			{
				obj
			};

			InvokeScriptMethod_Instance(*method, *instance, params);
		});

}

void Scene_DestroyEntity(UUID id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}


	Entity entity = s_MonoData.scene->GetEntity(id);
	if (!entity)
	{
		TRC_ERROR("Entity is presented in scene. Scene Name: {}", s_MonoData.scene->GetName());
		return;
	}

	s_MonoData.scene->DestroyEntity(entity);

}

void Scene_EnableEntity(UUID id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}


	Entity entity = s_MonoData.scene->GetEntity(id);
	if (!entity)
	{
		TRC_ERROR("Entity is presented in scene. Scene Name: {}", s_MonoData.scene->GetName());
		return;
	}

	s_MonoData.scene->EnableEntity(entity);

}

void Scene_DisableEntity(UUID id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}


	Entity entity = s_MonoData.scene->GetEntity(id);
	if (!entity)
	{
		TRC_ERROR("Entity is presented in scene. Scene Name: {}", s_MonoData.scene->GetName());
		return;
	}

	s_MonoData.scene->DisableEntity(entity);

}

MonoObject* Scene_GetEntityWithComponent(MonoReflectionType* reflect_type)
{
	MonoObject* result = (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return result;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	auto it = s_MonoData.get_entity_with_component.find(type);

	if (it == s_MonoData.get_entity_with_component.end())
	{
		TRC_WARN("Component is not valid, => {}", mono_type_get_name(type));
		return result;
	}
	
	Entity entity = it->second();
	if (entity)
	{
		result = (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(entity.GetID())->GetBackendHandle();
	}
	
	return result;
}

MonoObject* Scene_GetEntityWithScript(MonoReflectionType* reflect_type)
{
	MonoObject* result = (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return result;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);
	
	s_MonoData.scene->GetScriptRegistry().Iterate((uintptr_t)type, [&result](UUID id, Script* script, ScriptInstance* instance) 
		{
			result = (MonoObject*)instance->GetBackendHandle();
			return true;
		});

	return result;
}

bool Scene_GetStimulatePhysics()
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return false;
	}

	return s_MonoData.scene->GetStimulatePhysics();
}

void Scene_SetStimulatePhysics(bool value)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	s_MonoData.scene->SetStimulatePhysics(value);
}

#pragma endregion

#pragma region Physics

void Physics_GetCollisionData(CollisionData* out_data, int64_t collision_data)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}
	CollisionData* ptr = (CollisionData*)collision_data;
	
	out_data->entity = ptr->entity;
	out_data->otherEntity = ptr->otherEntity;
	out_data->impulse = ptr->impulse;
	out_data->numContacts = ptr->numContacts;

	for (uint32_t i = 0; i < ptr->numContacts; i++)
	{
		out_data->contacts[i].normal = ptr->contacts[i].normal;
		out_data->contacts[i].point = ptr->contacts[i].point;
		out_data->contacts[i].seperation = ptr->contacts[i].seperation;
	}

}

void Physics_GetTriggerData(TriggerPair* out_data, int64_t trigger_data)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}
	TriggerPair* ptr = (TriggerPair*)trigger_data;

	out_data->entity = ptr->entity;
	out_data->otherEntity = ptr->otherEntity;
	

}

void Physics_Step(float deltaTime)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	s_MonoData.scene->PhysicsStep(deltaTime);
}


#pragma endregion

#pragma region CharacterController

bool CharacterController_IsGrounded(UUID id)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return false;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return false;
	}

	CharacterControllerComponent& controller = entity.GetComponent<CharacterControllerComponent>();

	return controller.character.GetIsGrounded();
}

void CharacterController_Move(UUID id, glm::vec3* displacement, float deltaTime)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	CharacterControllerComponent& controller = entity.GetComponent<CharacterControllerComponent>();

	PhysicsFunc::MoveCharacterController(controller.character, *displacement, deltaTime);

}

void CharacterController_SetPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	if (CharacterControllerComponent* controller = entity.TryGetComponent<CharacterControllerComponent>())
	{
		PhysicsFunc::SetCharacterControllerPosition(controller->character, *position);
	}

}

void CharacterController_GetPosition(UUID id, glm::vec3* position)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	if (CharacterControllerComponent* controller = entity.TryGetComponent<CharacterControllerComponent>())
	{
		PhysicsFunc::GetCharacterControllerPosition(controller->character, *position);
	}

}



#pragma endregion

#pragma region AnimationGraphController

void AnimationGraphController_SetParameterBool(UUID id, MonoString* parameter_name, bool value)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(id);

	if (!entity)
	{
		TRC_ERROR("Invalid Entity, func:{}", __FUNCTION__);
		return;
	}

	AnimationGraphController& controller = entity.GetComponent<AnimationGraphController>();

	char* c_str = mono_string_to_utf8(parameter_name);

	controller.graph.SetParameterData(c_str, value);

	mono_free(c_str);

}

#pragma endregion

#pragma region Maths

void Maths_Quat_LookDirection(glm::vec3* direction, glm::quat* out_rotation)
{
	*out_rotation = glm::quatLookAt(-(*direction), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Maths_Quat_Mul_Vec(glm::quat* rotation, glm::vec3* direction, glm::vec3* out_direction)
{
	*out_direction = *rotation * *direction;
}

void Maths_Quat_Get_Euler_Angle(glm::quat* rotation, glm::vec3* out_angle)
{
	*out_angle = glm::degrees(glm::eulerAngles(*rotation));
}

void Maths_Quat_Set_Euler_Angle(glm::quat* out_rotation, glm::vec3* euler)
{
	*out_rotation = glm::quat(glm::radians(*euler));
}

void Maths_Quat_Slerp(glm::quat* a, glm::quat* b, float lerp_value, glm::quat* out_rotation)
{
	*out_rotation = glm::slerp(*a, *b, lerp_value);
}


#pragma endregion

#pragma region Application

bool Application_LoadAndSetScene(MonoString* filename)
{
	char* c_str = mono_string_to_utf8(filename);

	bool result = LoadAndSetScene(c_str);

	mono_free(c_str);
	return result;
}


#pragma endregion

#pragma region Stream

void Stream_WriteInt(uint64_t stream_handle, int value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(value);
}

void Stream_WriteFloat(uint64_t stream_handle, float value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(value);
}

void Stream_WriteVec2(uint64_t stream_handle, glm::vec2* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(*value);
}

void Stream_WriteVec3(uint64_t stream_handle, glm::vec3* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(*value);
}

void Stream_WriteQuat(uint64_t stream_handle, glm::quat* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(*value);
}

void Stream_ReadInt(uint64_t stream_handle, int* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
}

void Stream_ReadFloat(uint64_t stream_handle, float* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
}

void Stream_ReadVec2(uint64_t stream_handle, glm::vec2* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
}

void Stream_ReadVec3(uint64_t stream_handle, glm::vec3* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
}

void Stream_ReadQuat(uint64_t stream_handle, glm::quat* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
}


#pragma endregion

#pragma region Networking

bool Networking_IsServer()
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->IsServer();
}

bool Networking_IsClient()
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->IsClient();
}

void Networking_InvokeRPC(uint64_t uuid, MonoObject* src, uint64_t func_name_id, Network::RPCType type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Entity entity = s_MonoData.scene->GetEntity(uuid);
	if (!entity)
	{
		TRC_ERROR("Entity is not present in the current scene. Scene Name: {}, Function: {}", s_MonoData.scene->GetName(), __FUNCTION__);
		return;
	}

	ScriptEngine* engine = ScriptEngine::get_instance();
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	Network::NetType net_type = net_manager->GetNetType();
	uint32_t instance_id = net_manager->GetInstanceID();

	MonoType* class_type = mono_class_get_type(mono_object_get_class(src));

	ScriptInstance* instance = entity.GetScript((uintptr_t)class_type);
	if (!instance)
	{
		TRC_ASSERT(false, "These is not suppose to happen");
	}

	auto lambda = [&]()
	{
		
		trace::StringID string_id;
		string_id.value = func_name_id;
		ScriptMethod* method = instance->GetScript()->GetMethod(string_id);

		if (!method)
		{
			TRC_ASSERT(false, "These is not suppose to happen");
		}

		InvokeScriptMethod_Instance(*method, *instance, nullptr);
	};

	auto send_lambda = [&]()
	{
		Network::NetworkStream* data_stream = net_manager->GetRPCSendNetworkStream();
		Network::PacketMessageType message_type = Network::PacketMessageType::RPC;
		data_stream->Write(message_type);
		data_stream->Write(uuid);
		data_stream->Write(instance->GetScript()->GetScriptName());
		data_stream->Write(func_name_id);
		data_stream->Write(type);
		data_stream->Write(instance_id);
	};

	switch (net_type)
	{
	
	case Network::NetType::UNKNOWN:
	{
		lambda();
		break;
	}
	case Network::NetType::CLIENT:
	{
		if (type == Network::RPCType::CLIENT)
		{
			lambda();
		}

		send_lambda();
		break;
	}
	case Network::NetType::LISTEN_SERVER:
	{
		if (type == Network::RPCType::CLIENT)
		{
			lambda();
			send_lambda();
		}
		
		if (type == Network::RPCType::SERVER)
		{
			lambda();
		}
		break;
	}
	}


	

}

bool Networking_CreateListenServer(uint32_t port)
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();

	return net_manager->CreateListenServer(port);

}

bool Networking_CreateClient(bool LAN)
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();

	return net_manager->CreateClient(LAN);
}

bool Networking_ConnectTo(MonoString* server, uint32_t port)
{
	std::string txt;

	char* c_str = mono_string_to_utf8(server);
	txt = c_str;
	mono_free(c_str);

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->ConnectTo(txt, port);
}

bool Networking_ConnectToLAN(MonoString* server)
{
	std::string txt;

	char* c_str = mono_string_to_utf8(server);
	txt = c_str;
	mono_free(c_str);

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->ConnectToLAN(txt);
}

uint32_t Networking_InstanceID()
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->GetInstanceID();
}

void Networking_SendScenePacket(float deltaTime)
{
	//Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	s_MonoData.scene->OnNetworkUpdate(deltaTime);
}

#pragma endregion

#define ADD_INTERNAL_CALL(func) mono_add_internal_call("Trace.InternalCalls::"#func, &func)

void BindInternalFuncs()
{
	ADD_INTERNAL_CALL(Debug_Info);
	ADD_INTERNAL_CALL(Debug_Log);
	ADD_INTERNAL_CALL(Debug_Trace);
	ADD_INTERNAL_CALL(Debug_Sphere);

	ADD_INTERNAL_CALL(Action_GetComponent);
	ADD_INTERNAL_CALL(Action_GetScript);
	ADD_INTERNAL_CALL(Action_AddScript);
	ADD_INTERNAL_CALL(Action_HasComponent);
	ADD_INTERNAL_CALL(Action_HasScript);
	ADD_INTERNAL_CALL(Action_RemoveComponent);
	ADD_INTERNAL_CALL(Action_RemoveScript);
	ADD_INTERNAL_CALL(Action_GetName);
	ADD_INTERNAL_CALL(Action_IsOwner);
	ADD_INTERNAL_CALL(Action_GetNetID);


	ADD_INTERNAL_CALL(TransformComponent_GetPosition);
	ADD_INTERNAL_CALL(TransformComponent_SetPosition);
	ADD_INTERNAL_CALL(TransformComponent_GetWorldPosition);
	ADD_INTERNAL_CALL(TransformComponent_SetWorldPosition);
	ADD_INTERNAL_CALL(TransformComponent_GetWorldRotation);
	ADD_INTERNAL_CALL(TransformComponent_SetWorldRotation);
	ADD_INTERNAL_CALL(TransformComponent_GetRotation);
	ADD_INTERNAL_CALL(TransformComponent_SetRotation);
	ADD_INTERNAL_CALL(TransformComponent_Forward);
	ADD_INTERNAL_CALL(TransformComponent_Right);
	ADD_INTERNAL_CALL(TransformComponent_Up);

	ADD_INTERNAL_CALL(Input_GetKey);
	ADD_INTERNAL_CALL(Input_GetKeyPressed);
	ADD_INTERNAL_CALL(Input_GetKeyReleased);
	ADD_INTERNAL_CALL(Input_GetButton);
	ADD_INTERNAL_CALL(Input_GetButtonPressed);
	ADD_INTERNAL_CALL(Input_GetButtonReleased);

	ADD_INTERNAL_CALL(TextComponent_GetString);
	ADD_INTERNAL_CALL(TextComponent_SetString);

	ADD_INTERNAL_CALL(Scene_GetEntityByName);
	ADD_INTERNAL_CALL(Scene_GetEntity);
	ADD_INTERNAL_CALL(Scene_GetChildEntityByName);
	ADD_INTERNAL_CALL(Scene_InstanciateEntity_Position);
	ADD_INTERNAL_CALL(Scene_InstanciateEntity_Prefab_Position);
	ADD_INTERNAL_CALL(Scene_InstanciateEntity_Prefab_Position_NetID);
	ADD_INTERNAL_CALL(Scene_InstanciateEntity_Position_NetID);
	ADD_INTERNAL_CALL(Scene_DestroyEntity);
	ADD_INTERNAL_CALL(Scene_EnableEntity);
	ADD_INTERNAL_CALL(Scene_DisableEntity);
	ADD_INTERNAL_CALL(Scene_IterateComponent);
	ADD_INTERNAL_CALL(Scene_IterateEntityScripts);
	ADD_INTERNAL_CALL(Scene_GetEntityWithComponent);
	ADD_INTERNAL_CALL(Scene_GetEntityWithScript);
	ADD_INTERNAL_CALL(Scene_GetStimulatePhysics);
	ADD_INTERNAL_CALL(Scene_SetStimulatePhysics);

	ADD_INTERNAL_CALL(Physics_GetCollisionData);
	ADD_INTERNAL_CALL(Physics_GetTriggerData);
	ADD_INTERNAL_CALL(Physics_Step);

	ADD_INTERNAL_CALL(CharacterController_IsGrounded);
	ADD_INTERNAL_CALL(CharacterController_Move);
	ADD_INTERNAL_CALL(CharacterController_SetPosition);
	ADD_INTERNAL_CALL(CharacterController_GetPosition);

	ADD_INTERNAL_CALL(AnimationGraphController_SetParameterBool);

	ADD_INTERNAL_CALL(Maths_Quat_LookDirection);
	ADD_INTERNAL_CALL(Maths_Quat_Slerp);
	ADD_INTERNAL_CALL(Maths_Quat_Mul_Vec);
	ADD_INTERNAL_CALL(Maths_Quat_Get_Euler_Angle);
	ADD_INTERNAL_CALL(Maths_Quat_Set_Euler_Angle);

	ADD_INTERNAL_CALL(Application_LoadAndSetScene);

	ADD_INTERNAL_CALL(Stream_WriteInt);
	ADD_INTERNAL_CALL(Stream_WriteFloat);
	ADD_INTERNAL_CALL(Stream_WriteVec2);
	ADD_INTERNAL_CALL(Stream_WriteVec3);
	ADD_INTERNAL_CALL(Stream_WriteQuat);
	ADD_INTERNAL_CALL(Stream_ReadInt);
	ADD_INTERNAL_CALL(Stream_ReadFloat);
	ADD_INTERNAL_CALL(Stream_ReadVec2);
	ADD_INTERNAL_CALL(Stream_ReadVec3);
	ADD_INTERNAL_CALL(Stream_ReadQuat);

	ADD_INTERNAL_CALL(Networking_IsServer);
	ADD_INTERNAL_CALL(Networking_IsClient);
	ADD_INTERNAL_CALL(Networking_InvokeRPC);
	ADD_INTERNAL_CALL(Networking_CreateListenServer);
	ADD_INTERNAL_CALL(Networking_CreateClient);
	ADD_INTERNAL_CALL(Networking_ConnectTo);
	ADD_INTERNAL_CALL(Networking_ConnectToLAN);
	ADD_INTERNAL_CALL(Networking_InstanceID);
	ADD_INTERNAL_CALL(Networking_SendScenePacket);

}

bool Script_OnCollisionEnter(UUID entity_id, ScriptInstance& instance, CollisionData& collision_data)
{
	
	MonoInstance* internal_ins = (MonoInstance*)instance.GetInternal();


	void* ptr = &collision_data;
	int64_t ptr_location = (int64_t)ptr;
	MonoObject* exp = nullptr;
	void* args[] =
	{
		instance.GetBackendHandle(),
		&ptr_location
	};

	mono_runtime_invoke(s_MonoData.on_collision_enter, nullptr, args, &exp);
	if (exp) mono_print_unhandled_exception(exp);

	return true;
}

bool Script_OnCollisionExit(UUID entity_id, ScriptInstance& instance, CollisionData& collision_data)
{

	MonoInstance* internal_ins = (MonoInstance*)instance.GetInternal();

	void* ptr = &collision_data;
	int64_t ptr_location = (int64_t)ptr;
	MonoObject* exp = nullptr;
	void* args[] =
	{
		instance.GetBackendHandle(),
		&ptr_location
	};

	mono_runtime_invoke(s_MonoData.on_collision_exit, nullptr, args, &exp);
	if (exp) mono_print_unhandled_exception(exp);

	return true;
}

bool Script_OnTriggerEnter(UUID entity_id, ScriptInstance& instance, TriggerPair& pair)
{
	MonoInstance* internal_ins = (MonoInstance*)instance.GetInternal();

	void* ptr = &pair;
	int64_t ptr_location = (int64_t)ptr;
	MonoObject* exp = nullptr;
	void* args[] =
	{
		instance.GetBackendHandle(),
		&ptr_location
	};

	mono_runtime_invoke(s_MonoData.on_trigger_enter, nullptr, args, &exp);
	if (exp) mono_print_unhandled_exception(exp);

	return true;
}

bool Script_OnTriggerExit(UUID entity_id, ScriptInstance& instance, TriggerPair& pair)
{

	MonoInstance* internal_ins = (MonoInstance*)instance.GetInternal();

	void* ptr = &pair;
	int64_t ptr_location = (int64_t)ptr;
	MonoObject* exp = nullptr;
	void* args[] =
	{
		instance.GetBackendHandle(),
		&ptr_location
	};

	mono_runtime_invoke(s_MonoData.on_trigger_exit, nullptr, args, &exp);
	if (exp) mono_print_unhandled_exception(exp);

	return true;
}
