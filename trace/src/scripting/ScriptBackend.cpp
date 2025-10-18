#include "pch.h"


#include "ScriptBackend.h"
#include "core/FileSystem.h"
#include "core/io/Logging.h"
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
#include "core/events/EventsSystem.h"
#include "scripting/ScriptBackendTypes.h"


#include "spdlog/fmt/fmt.h"


#define MAX_METHOD_PARAM_COUNT 16



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

static std::unordered_map<ScriptFieldType, uint32_t> s_FieldTypesSize = 
{
	{ ScriptFieldType::Float, 4},
	{ ScriptFieldType::String, 0},
	{ ScriptFieldType::Int32, 4},
	{ ScriptFieldType::UInt32, 4},
	{ ScriptFieldType::Int64, 8},
	{ ScriptFieldType::UInt64, 8},
	{ ScriptFieldType::Double, 8},
	{ ScriptFieldType::Char, 1},
	{ ScriptFieldType::Int16, 2},
	{ ScriptFieldType::UInt16, 2},
	{ ScriptFieldType::Bool, 1},
	{ ScriptFieldType::Byte, 1},
	{ ScriptFieldType::Vec2, 8},
	{ ScriptFieldType::Vec3, 12},
	{ ScriptFieldType::Vec4, 16},
	{ ScriptFieldType::Action, 8},
	{ ScriptFieldType::Prefab, 8}
};


char* ReadBytes(const std::string& filepath, uint32_t* outSize);
MonoAssembly* LoadAssembly(const std::string& filePath);
void PrintAssemblyTypes(MonoAssembly* assembly);
void LoadAssemblyTypes(MonoAssembly* assembly, std::unordered_map<std::string, Script>& data, bool core);

void Script_OnEvent(Event* p_event);



bool InitializeInternal(const std::string& bin_dir)
{


	mono_set_assemblies_path(bin_dir.c_str());

	
	s_MonoData.mono_dir = bin_dir;
	s_MonoData.rootDomain = mono_jit_init("TraceScritingEngine");
	s_MonoData.appDomain = mono_domain_create_appdomain("TraceAppDomain", nullptr);
	mono_domain_set(s_MonoData.appDomain, true);

	BindInternalFuncs();

	EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_TYPED, Script_OnEvent);

	return true;
}
bool ShutdownInternal()
{
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(s_MonoData.appDomain);

	mono_jit_cleanup(s_MonoData.rootDomain);
	return true;
}


bool LoadComponents();

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

	s_MonoData.application_class = mono_class_from_name(s_MonoData.coreImage, "Trace", "Application");
	s_MonoData.invoke_key_typed = mono_class_get_method_from_name(s_MonoData.application_class, "InvokeKeyTyped", 1);


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

		MonoMethodSignature* sig = mono_method_signature(method);
		MonoType* param_type = nullptr;
		const char* name;

		void* iter = nullptr;
		std::vector<ScriptField> method_params;
		for (int i = 0; i < mono_signature_get_param_count(sig); i++)
		{
			ScriptField field_res;
			param_type = mono_signature_get_params(sig, &iter);
			std::string field_type = mono_type_get_name(param_type);
			if (s_FieldTypes.find(field_type) != s_FieldTypes.end())
			{
				field_res.field_type = s_FieldTypes.at(field_type);
			}
			else
			{
				field_res.field_type = ScriptFieldType::UnKnown;
			}

			method_params.push_back(field_res);
		}

		script.GetMethods()[STR_ID(method_name)].parameters = method_params;


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

void ReadMethodParams(ScriptMethod* method, void** out_params, glm::vec4* out_params_value, DataStream* data_stream)
{
	for (uint32_t i = 0; i < method->parameters.size(); i++)
	{
		ScriptField& field = method->parameters[i];

		switch (field.field_type)
		{
		case ScriptFieldType::Bool:
		{
			data_stream->Read(&out_params_value[i], sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Int32:
		{
			data_stream->Read(&out_params_value[i], sizeof(int32_t));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Float:
		{
			data_stream->Read(&out_params_value[i], sizeof(float));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec2:
		{
			data_stream->Read(&out_params_value[i], sizeof(glm::vec2));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec3:
		{
			data_stream->Read(&out_params_value[i], sizeof(glm::vec3));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec4:
		{
			data_stream->Read(&out_params_value[i], sizeof(glm::vec4));
			out_params[i] = &out_params_value[i];
			break;
		}
		}
	}
}

void WriteMethodParams(ScriptMethod* method, MonoArray* method_params, DataStream* data_stream)
{
	for (uint32_t i = 0; i < method->parameters.size(); i++)
	{
		ScriptField& field = method->parameters[i];
		MonoObject* val_i = mono_array_get(method_params, MonoObject*, i);

		switch (field.field_type)
		{
		case ScriptFieldType::Bool:
		{
			bool value = *(bool*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		case ScriptFieldType::Int32:
		{
			int32_t value = *(int32_t*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		case ScriptFieldType::Float:
		{
			float value = *(float*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		case ScriptFieldType::Vec2:
		{
			glm::vec2 value = *(glm::vec2*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		case ScriptFieldType::Vec3:
		{
			glm::vec3 value = *(glm::vec3*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		case ScriptFieldType::Vec4:
		{
			glm::vec4 value = *(glm::vec4*)mono_object_unbox(val_i);

			data_stream->Write(value);
			break;
		}
		}
	}
}

void GetMethodParams(ScriptMethod* method, MonoArray* method_params, void** out_params, glm::vec4* out_params_value)
{
	for (uint32_t i = 0; i < method->parameters.size(); i++)
	{
		ScriptField& field = method->parameters[i];
		MonoObject* val_i = mono_array_get(method_params, MonoObject*, i);

		switch (field.field_type)
		{
		case ScriptFieldType::Bool:
		{
			bool value = *(bool*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Int32:
		{
			int32_t value = *(int32_t*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Float:
		{
			float value = *(float*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec2:
		{
			glm::vec2 value = *(glm::vec2*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec3:
		{
			glm::vec3 value = *(glm::vec3*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		case ScriptFieldType::Vec4:
		{
			glm::vec4 value = *(glm::vec4*)mono_object_unbox(val_i);

			memcpy(&out_params_value[i], &value, sizeof(bool));
			out_params[i] = &out_params_value[i];
			break;
		}
		}
	}
}

void InvokeNetworkRPC(DataStream* rpc_data)
{

	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	Network::NetType net_type = net_manager->GetNetType();
	uint32_t instance_id = net_manager->GetInstanceID();

	UUID id = 0;
	rpc_data->Read(id);
	TRC_ASSERT(id != 0, "This is not suppose to happen");
	Entity entity = s_MonoData.scene->GetEntity(id);
	TRC_ASSERT(entity, "This is not suppose to happen");
	std::string script_name;
	rpc_data->Read(script_name);
	ScriptInstance* instance = entity.GetScript(script_name);
	TRC_ASSERT(instance, "This is not suppose to happen");
	uint64_t func_id = 0;
	rpc_data->Read(func_id);
	TRC_ASSERT(func_id != 0, "This is not suppose to happen");
	Network::RPCType rpc_type = Network::RPCType::UNKNOW;
	rpc_data->Read(rpc_type);
	uint32_t src_instance_id = 0;
	rpc_data->Read(src_instance_id);



	trace::StringID string_id;
	string_id.value = func_id;
	ScriptMethod* method = instance->GetScript()->GetMethod(string_id);

	TRC_ASSERT(method, "These is not suppose to happen");

	void* params[MAX_METHOD_PARAM_COUNT];
	glm::vec4 param_data[MAX_METHOD_PARAM_COUNT];

	ReadMethodParams(method, params, param_data, rpc_data);

	

	switch (net_type)
	{

	case Network::NetType::UNKNOWN:
	{
		break;
	}
	case Network::NetType::CLIENT:
	{
		if (rpc_type == Network::RPCType::CLIENT && instance_id != src_instance_id)
		{			
			InvokeScriptMethod_Instance(*method, *instance, params);
		}
		break;
	}
	case Network::NetType::LISTEN_SERVER:
	{
		if (rpc_type == Network::RPCType::SERVER)
		{
			InvokeScriptMethod_Instance(*method, *instance, params);
		}
		else if (rpc_type == Network::RPCType::CLIENT)
		{
			//TODO: Determine if it is listen server before you invoke the method
			InvokeScriptMethod_Instance(*method, *instance, params);
			// Send RPC to other clients
		}
		break;
	}
	}

}


void Script_OnEvent(Event* p_event)
{
	if (!s_MonoData.scene)
	{
		return;
	}

	switch (p_event->GetEventType())
	{
	case EventType::TRC_KEY_TYPED:
	{
		KeyTyped* typed = (KeyTyped*)p_event;
		int letter = (int)typed->GetKeyCode();

		void* params[] =
		{
			&letter
		};

		MonoObject* exp = nullptr;

		mono_runtime_invoke(s_MonoData.invoke_key_typed, nullptr, params, &exp);
		if (exp) mono_print_unhandled_exception(exp);

		break;
	}
	}


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

void Debug_SphereTimed(float duration, glm::vec3* position, float radius, uint32_t steps, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), *position);
	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->DrawDebugSphere_Timed(duration, radius, steps, transform, final_color);

}

void Debug_Line(glm::vec3* from, glm::vec3* to, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	glm::mat4 transform = glm::mat4(1.0f);
	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->AddDebugLine(*from, *to, transform, final_color);

}


void Debug_LineTimed(float duration, glm::vec3* from, glm::vec3* to, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	glm::mat4 transform = glm::mat4(1.0f);
	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->DrawLine_Timed(duration, *from, *to, transform, final_color);

}

void Debug_Box(glm::vec3* half_extents, glm::mat4* transform, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->DrawDebugBox(half_extents->x, half_extents->y, half_extents->z, *transform, final_color);

}

void Debug_BoxTimed(float duration, glm::vec3* half_extents, glm::mat4* transform, glm::vec3* color)
{
	trace::Debugger* debugger = trace::Debugger::get_instance();

	uint32_t final_color = colorVec4ToUint(glm::vec4(*color, 1.0f));

	debugger->DrawDebugBox_Timed(duration, *half_extents, *transform, final_color);

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

MonoObject* Action_GetParent(uint64_t entity_id)
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

	Entity parent = entity.GetParent();
	if (!parent)
	{
		TRC_ERROR("Entity dose not have a parent.Entity: {} Scene Name: {}, Function: {}", entity.GetName(), s_MonoData.scene->GetName(), __FUNCTION__);
		return (MonoObject*)ScriptEngine::get_instance()->GetEntityActionClass(0)->GetBackendHandle();
	}

	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(parent.GetID());

	return (MonoObject*)ins->GetBackendHandle();
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
	TransformComponent& transform = entity.GetComponent<TransformComponent>();
	transform._transform.SetPosition(*position);

	//Update Physics objects internal state
	if (CharacterControllerComponent* controller = entity.TryGetComponent<CharacterControllerComponent>())
	{
		PhysicsFunc::SetCharacterControllerPosition(controller->character, *position);
	}

	if (BoxColliderComponent* collider = entity.TryGetComponent<BoxColliderComponent>())
	{
		if (collider->is_trigger)
		{
			PhysicsFunc::UpdateShapeTransform(collider->_internal, collider->shape, transform._transform);
		}
	}
	
	if (SphereColliderComponent* collider = entity.TryGetComponent<SphereColliderComponent>())
	{
		if (collider->is_trigger)
		{
			PhysicsFunc::UpdateShapeTransform(collider->_internal, collider->shape, transform._transform);
		}
	}

	if (RigidBodyComponent* rigid_body = entity.TryGetComponent<RigidBodyComponent>())
	{
		PhysicsFunc::SetRigidBodyTransform(rigid_body->body, transform._transform);
	}

}

void TransformComponent_GetScale(UUID id, glm::vec3* scale)
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

	glm::vec3 res = entity.GetComponent<TransformComponent>()._transform.GetScale();
	*scale = res;
}

void TransformComponent_SetScale(UUID id, glm::vec3* scale)
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
	TransformComponent& transform = entity.GetComponent<TransformComponent>();
	transform._transform.SetScale(*scale);


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

void TransformComponent_MatrixTransform(UUID id, glm::mat4* result)
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

	*result = entity.GetComponent<TransformComponent>()._transform.GetLocalMatrix();
}

void TransformComponent_WorldMatrixTransform(UUID id, glm::mat4* result)
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

	*result = s_MonoData.scene->GetEntityGlobalPose(entity).GetLocalMatrix();
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

bool Input_GetGamepadKey(GamepadKeys key, int controller_id = 0)
{
	return InputSystem::get_instance()->GetGamepadKey(key, controller_id);
}

bool Input_GetGamepadKeyPressed(GamepadKeys key, int controller_id = 0)
{
	return InputSystem::get_instance()->GetGamepadKeyPressed(key, controller_id);
}

bool Input_GetGamepadKeyReleased(GamepadKeys key, int controller_id = 0)
{
	return InputSystem::get_instance()->GetGamepadKeyReleased(key, controller_id);
}

float Input_GetLeftStickX(int controller_id = 0)
{
	return InputSystem::get_instance()->GetLeftStickX(controller_id);
}

float Input_GetLeftStickY(int controller_id = 0)
{
	return InputSystem::get_instance()->GetLeftStickY(controller_id);
}

float Input_GetRightStickX(int controller_id = 0)
{
	return InputSystem::get_instance()->GetRightStickX(controller_id);
}

float Input_GetRightStickY(int controller_id = 0)
{
	return InputSystem::get_instance()->GetRightStickY(controller_id);
}

float Input_GetLeftTrigger(int controller_id = 0)
{
	return InputSystem::get_instance()->GetLeftTrigger(controller_id);
}

float Input_GetRightTrigger(int controller_id = 0)
{
	return InputSystem::get_instance()->GetRightTrigger(controller_id);
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

void TextComponent_GetColor(UUID id, glm::vec3* color)
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

	*color = entity.GetComponent<TextComponent>().color;

}

void TextComponent_SetColor(UUID id, glm::vec3* color)
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

	entity.GetComponent<TextComponent>().color = *color;

}

#pragma endregion

#pragma region RigidBody


void RigidBody_AddForce(UUID id, glm::vec3* force, ForceType mode)
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

	RigidBodyComponent& comp = entity.GetComponent<RigidBodyComponent>();

	PhysicsFunc::AddForce(comp.body, *force, mode);

}

void RigidBody_UpdateTransform(UUID id)
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

	RigidBodyComponent& comp = entity.GetComponent<RigidBodyComponent>();
	TransformComponent& transform = entity.GetComponent<TransformComponent>();

	PhysicsFunc::SetRigidBodyTransform(comp.body, transform._transform);

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
	TransformComponent_SetPosition(result.GetID(), position);

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
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());
	TransformComponent_SetPosition(result.GetID(), position);

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
	ScriptInstance* ins = ScriptEngine::get_instance()->GetEntityActionClass(result.GetID());
	TransformComponent_SetPosition(result.GetID(), position);

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

void Scene_IterateScript(UUID id, MonoObject* src, uint64_t func_name_id, MonoReflectionType* reflect_type)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	MonoType* type = mono_reflection_type_get_type(reflect_type);

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

	s_MonoData.scene->GetScriptRegistry().Iterate((uintptr_t)type, [method, instance](UUID id, Script* script, ScriptInstance* obj_instance)-> bool
		{
			MonoObject* obj = (MonoObject*)obj_instance->GetBackendHandle();

			void* params[] =
			{
				obj
			};

			InvokeScriptMethod_Instance(*method, *instance, params);

			return false;
		});
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

bool Physics_RayCast(glm::vec3* origin, glm::vec3* direction, float max_distance, RaycastHit* result)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return false;
	}

	return PhysicsFunc::RayCast(s_MonoData.scene->GetPhysics3D(), *origin, *direction, max_distance, *result);
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

void Stream_WriteBool(uint64_t stream_handle, bool value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Write(value);
}

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

void Stream_WriteString(uint64_t stream_handle, MonoString* value)
{
	DataStream* stream = (DataStream*)stream_handle;

	std::string txt;

	char* c_str = mono_string_to_utf8(value);
	txt = c_str;
	stream->Write(txt);
	mono_free(c_str);

}

void Stream_ReadBool(uint64_t stream_handle, bool* value)
{
	DataStream* stream = (DataStream*)stream_handle;
	stream->Read(*value);
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

MonoString* Stream_ReadString(uint64_t stream_handle)
{
	DataStream* stream = (DataStream*)stream_handle;
	std::string txt;
	stream->Read(txt);

	return mono_string_new(s_MonoData.appDomain, txt.c_str());
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

void Networking_InvokeRPC(uint64_t uuid, MonoObject* src, uint64_t func_name_id, Network::RPCType type, MonoArray* method_params)
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
	trace::StringID string_id;
	string_id.value = func_name_id;
	ScriptMethod* method = instance->GetScript()->GetMethod(string_id);

	auto lambda = [&]()
	{		

		if (!method)
		{
			TRC_ASSERT(false, "These is not suppose to happen");
		}

		void* params[MAX_METHOD_PARAM_COUNT];
		glm::vec4 param_data[MAX_METHOD_PARAM_COUNT];

		GetMethodParams(method, method_params, params, param_data);

		InvokeScriptMethod_Instance(*method, *instance, params);
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

		WriteMethodParams(method, method_params, data_stream);

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

float Networking_GetServerAverageRTT()
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->GetServerAverageRTT();
}

float Networking_GetClientAverageRTT(uint32_t client_id)
{
	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	return net_manager->GetClientAverageRTT(client_id);
}

MonoString* Networking_GetInstanceName()
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	
	std::string& txt = net_manager->GetInstanceName();

	if (txt.empty()) return nullptr;

	return mono_string_new(s_MonoData.appDomain, txt.c_str());

}

void Networking_SetInstanceName(MonoString* string)
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return;
	}

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
	std::string& txt = net_manager->GetInstanceName();

	char* c_str = mono_string_to_utf8(string);
	txt = c_str;
	mono_free(c_str);

}

MonoArray* Networking_GetFoundConnections()
{
	if (!s_MonoData.scene)
	{
		TRC_WARN("Scene is not yet valid, Function: {}", __FUNCTION__);
		return nullptr;
	}

	Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();

	auto found_connections = net_manager->GetClientFoundConnections();
	if (found_connections.empty())
	{
		return nullptr;
	}

	MonoClass* string_class = mono_get_string_class();
	MonoArray* arr_result = mono_array_new(s_MonoData.appDomain, string_class, found_connections.size());

	int32_t index = 0;
	for (auto& i : found_connections)
	{
		std::string txt = i.first;

		MonoString* server_name = mono_string_new(s_MonoData.appDomain, txt.c_str());

		mono_array_setref(arr_result, index, server_name);

		index++;
	}

	return arr_result;
}

#pragma endregion

#define ADD_INTERNAL_CALL(func) mono_add_internal_call("Trace.InternalCalls::"#func, &func)

void BindInternalFuncs()
{
	ADD_INTERNAL_CALL(Debug_Info);
	ADD_INTERNAL_CALL(Debug_Log);
	ADD_INTERNAL_CALL(Debug_Trace);
	ADD_INTERNAL_CALL(Debug_Sphere);
	ADD_INTERNAL_CALL(Debug_SphereTimed);
	ADD_INTERNAL_CALL(Debug_Line);
	ADD_INTERNAL_CALL(Debug_LineTimed);
	ADD_INTERNAL_CALL(Debug_Box);
	ADD_INTERNAL_CALL(Debug_BoxTimed);

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
	ADD_INTERNAL_CALL(Action_GetParent);


	ADD_INTERNAL_CALL(TransformComponent_GetPosition);
	ADD_INTERNAL_CALL(TransformComponent_SetPosition);
	ADD_INTERNAL_CALL(TransformComponent_GetScale);
	ADD_INTERNAL_CALL(TransformComponent_SetScale);
	ADD_INTERNAL_CALL(TransformComponent_GetWorldPosition);
	ADD_INTERNAL_CALL(TransformComponent_SetWorldPosition);
	ADD_INTERNAL_CALL(TransformComponent_GetWorldRotation);
	ADD_INTERNAL_CALL(TransformComponent_SetWorldRotation);
	ADD_INTERNAL_CALL(TransformComponent_GetRotation);
	ADD_INTERNAL_CALL(TransformComponent_SetRotation);
	ADD_INTERNAL_CALL(TransformComponent_Forward);
	ADD_INTERNAL_CALL(TransformComponent_Right);
	ADD_INTERNAL_CALL(TransformComponent_Up);
	ADD_INTERNAL_CALL(TransformComponent_MatrixTransform);
	ADD_INTERNAL_CALL(TransformComponent_WorldMatrixTransform);

	ADD_INTERNAL_CALL(Input_GetKey);
	ADD_INTERNAL_CALL(Input_GetKeyPressed);
	ADD_INTERNAL_CALL(Input_GetKeyReleased);
	ADD_INTERNAL_CALL(Input_GetGamepadKey);
	ADD_INTERNAL_CALL(Input_GetGamepadKeyPressed);
	ADD_INTERNAL_CALL(Input_GetGamepadKeyReleased);
	ADD_INTERNAL_CALL(Input_GetLeftStickX);
	ADD_INTERNAL_CALL(Input_GetLeftStickY);
	ADD_INTERNAL_CALL(Input_GetRightStickX);
	ADD_INTERNAL_CALL(Input_GetRightStickY);
	ADD_INTERNAL_CALL(Input_GetLeftTrigger);
	ADD_INTERNAL_CALL(Input_GetRightTrigger);
	ADD_INTERNAL_CALL(Input_GetButton);
	ADD_INTERNAL_CALL(Input_GetButtonPressed);
	ADD_INTERNAL_CALL(Input_GetButtonReleased);

	ADD_INTERNAL_CALL(TextComponent_GetString);
	ADD_INTERNAL_CALL(TextComponent_SetString);
	ADD_INTERNAL_CALL(TextComponent_GetColor);
	ADD_INTERNAL_CALL(TextComponent_SetColor);

	ADD_INTERNAL_CALL(RigidBody_AddForce);
	ADD_INTERNAL_CALL(RigidBody_UpdateTransform);

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
	ADD_INTERNAL_CALL(Scene_IterateScript);
	ADD_INTERNAL_CALL(Scene_IterateEntityScripts);
	ADD_INTERNAL_CALL(Scene_GetEntityWithComponent);
	ADD_INTERNAL_CALL(Scene_GetEntityWithScript);
	ADD_INTERNAL_CALL(Scene_GetStimulatePhysics);
	ADD_INTERNAL_CALL(Scene_SetStimulatePhysics);

	ADD_INTERNAL_CALL(Physics_GetCollisionData);
	ADD_INTERNAL_CALL(Physics_GetTriggerData);
	ADD_INTERNAL_CALL(Physics_Step);
	ADD_INTERNAL_CALL(Physics_RayCast);

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

	ADD_INTERNAL_CALL(Stream_WriteBool);
	ADD_INTERNAL_CALL(Stream_WriteInt);
	ADD_INTERNAL_CALL(Stream_WriteFloat);
	ADD_INTERNAL_CALL(Stream_WriteVec2);
	ADD_INTERNAL_CALL(Stream_WriteVec3);
	ADD_INTERNAL_CALL(Stream_WriteQuat);
	ADD_INTERNAL_CALL(Stream_WriteString);

	ADD_INTERNAL_CALL(Stream_ReadBool);
	ADD_INTERNAL_CALL(Stream_ReadInt);
	ADD_INTERNAL_CALL(Stream_ReadFloat);
	ADD_INTERNAL_CALL(Stream_ReadVec2);
	ADD_INTERNAL_CALL(Stream_ReadVec3);
	ADD_INTERNAL_CALL(Stream_ReadQuat);
	ADD_INTERNAL_CALL(Stream_ReadString);

	ADD_INTERNAL_CALL(Networking_IsServer);
	ADD_INTERNAL_CALL(Networking_IsClient);
	ADD_INTERNAL_CALL(Networking_InvokeRPC);
	ADD_INTERNAL_CALL(Networking_CreateListenServer);
	ADD_INTERNAL_CALL(Networking_CreateClient);
	ADD_INTERNAL_CALL(Networking_ConnectTo);
	ADD_INTERNAL_CALL(Networking_ConnectToLAN);
	ADD_INTERNAL_CALL(Networking_InstanceID);
	ADD_INTERNAL_CALL(Networking_SendScenePacket);
	ADD_INTERNAL_CALL(Networking_GetServerAverageRTT);
	ADD_INTERNAL_CALL(Networking_GetClientAverageRTT);
	ADD_INTERNAL_CALL(Networking_GetInstanceName);
	ADD_INTERNAL_CALL(Networking_SetInstanceName);
	ADD_INTERNAL_CALL(Networking_GetFoundConnections);

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
