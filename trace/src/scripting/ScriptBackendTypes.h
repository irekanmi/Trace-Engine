#pragma once

#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scripting/Script.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/threads.h"
#include <map>
#include <unordered_map>
#include <functional>

using namespace trace;

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

	MonoClass* application_class = nullptr;
	MonoMethod* invoke_key_typed = nullptr;
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
