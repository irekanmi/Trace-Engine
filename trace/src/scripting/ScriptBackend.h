#pragma once


#include "Script.h"
#include <string>

namespace trace {
	class Scene;
}

using namespace trace;

bool InitializeInternal(const std::string& bin_dir);
bool ShutdownInternal();
bool LoadComponents();

bool LoadCoreAssembly(const std::string& file_path);
bool LoadMainAssembly(const std::string& file_path);

bool LoadAllScripts(std::unordered_map<std::string, Script>& out_data);
bool ReloadAssemblies(const std::string& core_assembly, const std::string& main_assembly);

bool OnSceneStartInternal(Scene* scene);
bool OnSceneStopInternal(Scene* scene);

bool CreateScript(const std::string& name, Script& script, const std::string& nameSpace = "", bool core = false);
bool GetScriptID(Script& script, uintptr_t& res);
bool DestroyScript(Script& script);

bool CreateScriptInstance(Script& script, ScriptInstance& out_instance);
bool CreateScriptInstanceNoInit(Script& script, ScriptInstance& out_instance);
bool DestroyScriptInstance(ScriptInstance& instance);
bool GetScriptInstanceHandle(ScriptInstance& instance, void*& out);

bool GetScriptMethod(const std::string& method, ScriptMethod& out_method, Script& script, int param_count);
bool InvokeScriptMethod_Instance(ScriptMethod& method, ScriptInstance& instance, void** params);
bool InvokeScriptMethod_Class(ScriptMethod& method, Script& script, void** params);

bool GetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* out_value, uint32_t val_size);
bool SetInstanceFieldValue(ScriptInstance& instance, ScriptField& field, void* value, uint32_t val_size);

template<typename T>
void* GetValueInternal(T* value);


void BindInternalFuncs();





