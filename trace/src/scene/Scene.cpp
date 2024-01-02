#include "pch.h"

#include "Entity.h"
#include "Scene.h"
#include "Components.h"
#include "render/Renderer.h"
#include "backends/Physicsutils.h"
#include "scripting/Script.h"
#include "scripting/ScriptBackend.h"
#include "scripting/ScriptEngine.h"

namespace trace {
	void Scene::Create()
	{
		m_scriptRegistry.Init();
	}
	void Scene::Destroy()
	{
		m_registry.clear();
		m_scriptRegistry.Clear();
	}
	void Scene::OnStart()
	{
		PhysicsFunc::CreateScene3D(m_physics3D, glm::vec3(0.0f, -9.81f, 0.0f));
		if (m_physics3D)
		{
			auto rigid_view = m_registry.view<TransformComponent, RigidBodyComponent>();
			for (auto i : rigid_view)
			{
				auto [pose, rigid] = rigid_view.get(i);
				PhysicsFunc::CreateRigidBody_Scene(m_physics3D,rigid.body, pose._transform);
			}

			auto bcd = m_registry.view<TransformComponent, BoxColliderComponent>();
			for (auto i : bcd)
			{
				auto [pose, box] = bcd.get(i);
				glm::vec3 extent = box.shape.box.half_extents;
				box.shape.box.half_extents *= pose._transform.GetScale();
				Transform local;
				local.SetPosition(pose._transform.GetPosition() + box.shape.offset);
				local.SetRotation(pose._transform.GetRotation());
				PhysicsFunc::CreateShapeWithTransform(m_physics3D,box._internal, box.shape, local, box.is_trigger);
				//Temp _______________
				PhysicsFunc::SetShapeMask(box._internal, BIT(1), BIT(1));
				// -------------------

				box.shape.box.half_extents = extent;


				Entity entity(i, this);
				UUID _id = entity.GetComponent<IDComponent>()._id;
				Entity* shp_ptr = &m_entityMap[_id];
				PhysicsFunc::SetShapePtr(box._internal, shp_ptr);
				if (!box.is_trigger && entity.HasComponent<RigidBodyComponent>())
				{
					RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
					PhysicsFunc::SetRigidBodyTransform(rigid.body, local);
					PhysicsFunc::AttachShape(box._internal, rigid.body.GetInternal());

				}

			}

			auto scd = m_registry.view<TransformComponent, SphereColliderComponent>();
			for (auto i : scd)
			{
				auto [pose, sc] = scd.get(i);
				float radius = sc.shape.sphere.radius;
				sc.shape.sphere.radius *= pose._transform.GetScale().x; //TODO: Determine maybe scale in transform should be used or not
				Transform local;
				local.SetPosition(pose._transform.GetPosition() + sc.shape.offset);
				local.SetRotation(pose._transform.GetRotation());
				PhysicsFunc::CreateShapeWithTransform(m_physics3D, sc._internal, sc.shape, local, sc.is_trigger);
				sc.shape.sphere.radius = radius;
				//Temp _______________
				PhysicsFunc::SetShapeMask(sc._internal, BIT(1), BIT(1));
				// -------------------

				Entity entity(i, this);
				UUID _id = entity.GetComponent<IDComponent>()._id;
				Entity* shp_ptr = &m_entityMap[_id];
				PhysicsFunc::SetShapePtr(sc._internal, shp_ptr);
				if (!sc.is_trigger && entity.HasComponent<RigidBodyComponent>())
				{
					
					RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
					PhysicsFunc::SetRigidBodyTransform(rigid.body, local);
					PhysicsFunc::AttachShape(sc._internal, rigid.body.GetInternal());

				}

			}
		}
	}
	void Scene::OnScriptStart()
	{
		ScriptEngine::get_instance()->OnSceneStart(this);

		m_scriptRegistry.Iterate([](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* constructor = ScriptEngine::get_instance()->GetConstructor();
				auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();

				uint32_t index = 0;
				for (ScriptInstance& i : manager.instances)
				{
					CreateScriptInstance(*manager.script, i);

					// Setting values from the editor to the scene runtime..............
					auto it = fields_instances.find(manager.script);
					if (it != fields_instances.end())
					{
						auto field_it = it->second.find(manager.entities[index]);
						if (field_it != it->second.end())
						{
							for (auto& [name, data] : field_it->second.m_fields)
							{
								if (data.type == ScriptFieldType::String) continue;
								i.SetFieldValueInternal(name, data.data, 16);
							}
						}
					}
					// ..................................................................

					void* params[1] =
					{
						&manager.entities[index]
					};
					InvokeScriptMethod_Instance(*constructor, i, params);
					++index;
				}

			});

		m_scriptRegistry.Iterate([](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* on_start = manager.script->GetMethod("OnStart");
				if (!on_start) return;

				for (ScriptInstance& i : manager.instances)
				{
					InvokeScriptMethod_Instance(*on_start, i, nullptr);
				}

			});

		m_scriptRegistry.Iterate([](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* on_create = manager.script->GetMethod("OnCreate");
				if (!on_create) return;

				for (ScriptInstance& i : manager.instances)
				{
					InvokeScriptMethod_Instance(*on_create, i, nullptr);
				}

			});

	}
	void Scene::OnStop()
	{
		if (m_physics3D)
		{

			auto bodies = m_registry.view<RigidBodyComponent>();
			for (auto i : bodies)
			{
				auto [ rigid] = bodies.get(i);

				PhysicsFunc::RemoveActor(m_physics3D, rigid.body.GetInternal());
				PhysicsFunc::DestroyRigidBody(rigid.body);
			}

			auto box_coillders = m_registry.view<BoxColliderComponent>();
			for (auto i : box_coillders)
			{
				auto [box] = box_coillders.get(i);
				PhysicsFunc::DestroyShape(box._internal);

			}

			auto scd = m_registry.view<SphereColliderComponent>();
			for (auto i : scd)
			{
				auto [sc] = scd.get(i);
				PhysicsFunc::DestroyShape(sc._internal);

			}

			PhysicsFunc::DestroyScene3D(m_physics3D);
		}
	}
	void Scene::OnScriptStop()
	{

		m_scriptRegistry.Iterate([](ScriptRegistry::ScriptManager& manager)
			{

				for (ScriptInstance& i : manager.instances)
				{
					DestroyScriptInstance(i);
				}

			});

		ScriptEngine::get_instance()->OnSceneStop(this);

	}
	void Scene::OnUpdate(float deltaTime)
	{
		auto process_hierachy_transforms = [](Entity entity, UUID, Scene* scene)
		{
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			TransformComponent& transform = entity.GetComponent<TransformComponent>();
			Entity parent = scene->GetEntity(hi.parent);
			if (parent)
			{
				hi.transform = parent.GetComponent<HierachyComponent>().transform * transform._transform.GetLocalMatrix();
			}
			else
			{
				hi.transform = transform._transform.GetLocalMatrix();
			}

		};

		ProcessEntitiesByHierachy(process_hierachy_transforms);

	}

	void Scene::OnScriptUpdate(float deltaTime)
	{
		m_scriptRegistry.Iterate([deltaTime](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* on_update = manager.script->GetMethod("OnUpdate");

				if (!on_update) return;
				float dt = deltaTime;

				for (ScriptInstance& i : manager.instances)
				{
					void* params[1] =
					{
						&dt
					};
					InvokeScriptMethod_Instance(*on_update, i, params);
				}

			});
	}

	void Scene::OnPhysicsUpdate(float deltaTime)
	{
		if (m_physics3D)
		{
			auto bodies = m_registry.view<TransformComponent, RigidBodyComponent>();
			for (auto i : bodies)
			{
				auto [transform, rigid] = bodies.get(i);

				PhysicsFunc::SetRigidBodyTransform(rigid.body, transform._transform);
			}

			auto bcd = m_registry.view<TransformComponent, BoxColliderComponent>();
			for (auto i : bcd)
			{
				auto [pose, bc] = bcd.get(i);
				PhysicsFunc::UpdateShapeTransform(bc._internal, pose._transform);
			}

			auto scd = m_registry.view<TransformComponent, SphereColliderComponent>();
			for (auto i : scd)
			{
				auto [pose, sc] = scd.get(i);
				PhysicsFunc::UpdateShapeTransform(sc._internal, pose._transform);
			}
			

			PhysicsFunc::Stimulate(m_physics3D, deltaTime);
			for (auto i : bodies)
			{
				auto [transform ,rigid] = bodies.get(i);

				PhysicsFunc::GetRigidBodyTransform(rigid.body, transform._transform);
			}
		}
	}


	void Scene::OnRender()
	{
		Camera* main_camera = nullptr;

		auto view = m_registry.view<CameraComponent , TransformComponent>();
	
		for (auto entity : view)
		{
			auto [camera, _transform] = view.get<CameraComponent, TransformComponent>(entity);
			if (camera.is_main)
			{
				camera._camera.SetPosition(_transform._transform.GetPosition());
				main_camera = &camera._camera;
				break;
			}
		}

		if (main_camera)
		{
			Renderer* renderer = Renderer::get_instance();
			CommandList cmd_list = renderer->BeginCommandList();
			renderer->BeginScene(cmd_list, main_camera);

			OnRender(cmd_list);

			renderer->EndScene(cmd_list);

			renderer->SubmitCommandList(cmd_list);
		}

	}

	void Scene::OnRender(CommandList& cmd_list)
	{

		Renderer* renderer = Renderer::get_instance();

		auto light_group = m_registry.view<LightComponent, TransformComponent>();

		for (auto entity : light_group)
		{
			auto [light, transform] = light_group.get(entity);
			light._light.position = glm::vec4(transform._transform.GetPosition(), 0.0f);

			renderer->AddLight(cmd_list, light._light, light.light_type);

		}

		auto group = m_registry.group<MeshComponent, HierachyComponent>();

		for (auto entity : group)
		{
			auto [mesh, transform] = group.get(entity);

			renderer->DrawMesh(cmd_list, mesh._mesh, transform.transform); // TODO Implement Hierachies

		}

		auto model_view = m_registry.view<ModelComponent, ModelRendererComponent, HierachyComponent>();

		for (auto entity : model_view)
		{
			auto [model, model_renderer, transform] = model_view.get(entity);

			renderer->DrawModel(cmd_list, model._model, model_renderer._material, transform.transform); // TODO Implement Hierachies

		}

		auto text_view = m_registry.view<TextComponent, HierachyComponent>();

		for (auto entity : text_view)
		{
			auto [txt, transform] = text_view.get(entity);

			glm::vec3 color = txt.color * txt.intensity;
			renderer->DrawString(cmd_list, txt.font, txt.text, color, transform.transform); // TODO Implement Hierachies

		}

	}

	void Scene::OnViewportChange(float width, float height)
	{
		auto view = m_registry.view<CameraComponent>();

		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camera._camera.SetAspectRatio(width / height);
		}
	}


	Entity Scene::CreateEntity()
	{
		return CreateEntity_UUID(UUID::GenUUID(), "");
	}

	Entity Scene::CreateEntity(const std::string& _tag)
	{
		return CreateEntity_UUID(UUID::GenUUID(), _tag);
	}

	Entity Scene::CreateEntity_UUID(UUID id, const std::string& _tag)
	{
		return CreateEntity_UUIDWithParent(id, _tag, 0);
	}

	Entity Scene::CreateEntity_UUIDWithParent(UUID id, const std::string& _tag, UUID parent)
	{
		entt::entity handle = m_registry.create();
		Entity entity(handle, this);
		TagComponent& tag = entity.AddComponent<TagComponent>();
		tag._tag = _tag.empty() ? "New Entity" : _tag;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<IDComponent>()._id = id;
		m_entityMap[id] = entity;
		HierachyComponent& hi = entity.AddComponent<HierachyComponent>();
		if(parent == 0) m_rootNode.AddChid(id);		
		hi.parent = parent;

		return entity;
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		auto it = m_entityMap.find(uuid);
		if (it != m_entityMap.end()) return it->second;
		return Entity();
	}

	template<typename Component>
	void CopyComponent(Entity from, Entity to)
	{
		if (from.HasComponent<Component>())
		{
			to.AddOrReplaceComponent<Component>(from.GetComponent<Component>());
		}
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity res = CreateEntity_UUID(UUID::GenUUID(), entity.GetComponent<TagComponent>()._tag);

		CopyComponent<TagComponent>(entity, res);
		CopyComponent<TransformComponent>(entity, res);
		CopyComponent<CameraComponent>(entity, res);
		CopyComponent<LightComponent>(entity, res);
		CopyComponent<MeshComponent>(entity, res);
		CopyComponent<ModelComponent>(entity, res);
		CopyComponent<ModelRendererComponent>(entity, res);
		CopyComponent<TextComponent>(entity, res);
		CopyComponent<RigidBodyComponent>(entity, res);
		CopyComponent<BoxColliderComponent>(entity, res);
		CopyComponent<SphereColliderComponent>(entity, res);
		CopyComponent<HierachyComponent>(entity, res);

		m_scriptRegistry.Iterate(entity.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = res.AddScript(script->GetID());
				*sc_ins = *other;
			});

	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_scriptRegistry.Erase(entity.GetID());
		m_entityMap.erase(entity.GetComponent<IDComponent>()._id);
		m_registry.destroy(entity);
	}

	void Scene::ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback)
	{
		for (UUID& uuid : m_rootNode.children)
		{
			Entity entity = GetEntity(uuid);
			callback(entity, uuid, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback);
		}
	}

	template<typename Component>
	void CopyComponent(entt::registry& from, entt::registry& to, std::unordered_map<UUID, entt::entity>& entity_map)
	{
		auto view = from.view<Component>();

		for (auto e : view)
		{
			UUID uuid = from.get<IDComponent>(e)._id;
			auto [comp] = view.get(e);
			entt::entity en = entity_map[uuid];
			to.emplace_or_replace<Component>(en, comp);
		}

	}



	void Scene::Copy(Ref<Scene> from, Ref<Scene> to)
	{

		entt::registry& f_reg = from->m_registry;
		entt::registry& t_reg = to->m_registry;
		t_reg.clear();

		std::unordered_map<UUID, entt::entity> entity_map;

		auto id_view = f_reg.view<IDComponent>();
		for (auto e : id_view)
		{
			UUID uuid = id_view.get<IDComponent>(e)._id;
			Entity en = to->CreateEntity_UUID(uuid, "");
			entity_map[uuid] = en;
		}

		CopyComponent<TagComponent>(f_reg, t_reg, entity_map);
		CopyComponent<HierachyComponent>(f_reg, t_reg, entity_map);
		CopyComponent<TransformComponent>(f_reg, t_reg, entity_map);
		CopyComponent<CameraComponent>(f_reg, t_reg, entity_map);
		CopyComponent<LightComponent>(f_reg, t_reg, entity_map);
		CopyComponent<MeshComponent>(f_reg, t_reg, entity_map);
		CopyComponent<ModelComponent>(f_reg, t_reg, entity_map);
		CopyComponent<ModelRendererComponent>(f_reg, t_reg, entity_map);
		CopyComponent<TextComponent>(f_reg, t_reg, entity_map);
		CopyComponent<RigidBodyComponent>(f_reg, t_reg, entity_map);
		CopyComponent<BoxColliderComponent>(f_reg, t_reg, entity_map);
		CopyComponent<SphereColliderComponent>(f_reg, t_reg, entity_map);

		ScriptRegistry::Copy(from->m_scriptRegistry, to->m_scriptRegistry);

		to->m_rootNode = from->m_rootNode;

	}

	void Scene::ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback)
	{
		for (auto& i : hierachy.children)
		{
			Entity entity = GetEntity(i);
			callback(entity, i, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback);
		}
	}

}