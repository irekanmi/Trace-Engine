#include "pch.h"

#include "scene/SceneUtils.h"
#include "scene/Components.h"

namespace trace {
	
	template<typename... Component>
	void remove_component(Entity entity)
	{

		([&]() {
			if (entity.HasComponent<Component>())
			{
				entity.RemoveComponent<Component>();
			}
			}(), ...);


	}

	template<typename... Component>
	void remove_component(ComponentGroup<Component...>, Entity entity)
	{
		remove_component<Component...>(entity);
	}

	void remove_entity_physics_components(Entity entity)
	{
		remove_component(PhysicsComponents{}, entity);
	}

}
