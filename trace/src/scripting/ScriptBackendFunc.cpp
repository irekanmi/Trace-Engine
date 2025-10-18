#include "pch.h"

#include "scripting/ScriptBackendTypes.h"
#include "scripting/ScriptBackend.h"
#include "scene/Components.h"
#include "scripting/ScriptEngine.h"


extern MonoData s_MonoData;

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
void RegisterComponent(trace::ComponentGroup<Component...>)
{
	RegisterComponent<Component...>();
}

bool LoadComponents()
{

	RegisterComponent(AllComponents{});

	return true;
}