#include "pch.h"

#include "Entity.h"
#include "Scene.h"
#include "Components.h"
#include "render/Renderer.h"
#include "backends/Physicsutils.h"
#include "scripting/Script.h"
#include "scripting/ScriptBackend.h"
#include "scripting/ScriptEngine.h"
#include "animation/AnimationEngine.h"
#include "resource/PrefabManager.h"

#include "glm/gtx/matrix_decompose.hpp"

namespace trace {
	void Scene::Create()
	{
		m_scriptRegistry.Init();
		m_rootNode = new HierachyComponent(); // TODO: Use Custom Allocator

		m_registry.on_construct<HierachyComponent>().connect<&Scene::OnConstructHierachyComponent>(*this);
		m_registry.on_destroy<HierachyComponent>().connect<&Scene::OnDestroyHierachyComponent>(*this);

	}
	void Scene::Destroy()
	{
		m_registry.on_construct<HierachyComponent>().disconnect<&Scene::OnConstructHierachyComponent>(*this);
		m_registry.on_destroy<HierachyComponent>().disconnect<&Scene::OnDestroyHierachyComponent>(*this);
		delete m_rootNode; // TODO: Use Custom Allocator
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
		m_running = true;
		auto animations = m_registry.view<AnimationComponent>();
		for (auto i : animations)
		{
			auto [anim_comp] = animations.get(i);
			if (!anim_comp.anim_graph) continue;
			anim_comp.anim_graph->SetCurrentStateIndex(anim_comp.anim_graph->GetCurrentStateIndex() == -1 ? anim_comp.anim_graph->GetStartIndex() : anim_comp.anim_graph->GetCurrentStateIndex());
			AnimationState& current_state = anim_comp.anim_graph->GetStates()[anim_comp.anim_graph->GetCurrentStateIndex()];
			if (!current_state.GetAnimationClip()) continue;
			if (anim_comp.play_on_start) current_state.Play();
		}

	}
	void Scene::OnScriptStart()
	{
		ScriptEngine::get_instance()->OnSceneStart(this);

		ScriptRegistry& reg = m_scriptRegistry;

		m_scriptRegistry.Iterate([&reg](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* constructor = ScriptEngine::get_instance()->GetConstructor();
				auto& fields_instances = reg.GetFieldInstances();

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
							for (auto& [name, data] : field_it->second.GetFields())
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
		auto animations = m_registry.view<AnimationComponent>();
		for (auto i : animations)
		{
			auto [anim_comp] = animations.get(i);
			if (!anim_comp.anim_graph) continue;
			anim_comp.anim_graph->SetCurrentStateIndex(anim_comp.anim_graph->GetCurrentStateIndex() == -1 ? anim_comp.anim_graph->GetStartIndex() : anim_comp.anim_graph->GetCurrentStateIndex());
			AnimationState& current_state = anim_comp.anim_graph->GetStates()[anim_comp.anim_graph->GetCurrentStateIndex()];
			if (!current_state.GetAnimationClip()) continue;
			current_state.Stop();
		}

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
		m_running = false;
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
		ResolveHierachyTransforms();

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

	void Scene::OnAnimationUpdate(float deltaTime)
	{
		auto animations = m_registry.view<AnimationComponent>();
		for (auto i : animations)
		{
			auto [anim_comp] = animations.get(i);
			if (!anim_comp.anim_graph) continue;
			anim_comp.anim_graph->SetCurrentStateIndex(anim_comp.anim_graph->GetCurrentStateIndex() == -1 ? anim_comp.anim_graph->GetStartIndex() : anim_comp.anim_graph->GetCurrentStateIndex());
			AnimationState& current_state = anim_comp.anim_graph->GetStates()[anim_comp.anim_graph->GetCurrentStateIndex()];
			if (!current_state.GetAnimationClip()) continue;
			AnimationEngine::get_instance()->Animate(current_state, this);
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
				glm::vec3 forward = _transform._transform.GetForward();
				glm::vec3 up = _transform._transform.GetUp();
				glm::vec3 right = _transform._transform.GetRight();
				camera._camera.SetLookDir(forward);
				camera._camera.SetUpDir(up);
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
			light._light.direction = glm::vec4(transform._transform.GetForward(), 0.0f);

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

		auto images_view = m_registry.view<ImageComponent, HierachyComponent>();

		for (auto entity : images_view)
		{
			auto [img, transform] = images_view.get(entity);

			if(img.image) renderer->DrawImage(cmd_list, img.image, transform.transform); // TODO Implement Hierachies

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


	Entity Scene::CreateEntity(UUID parent)
	{
		return CreateEntity_UUID(UUID::GenUUID(), "", parent);
	}

	Entity Scene::CreateEntity(const std::string& _tag, UUID parent)
	{
		return CreateEntity_UUID(UUID::GenUUID(), _tag, parent);
	}

	Entity Scene::CreateEntity_UUID(UUID id, const std::string& _tag, UUID parent)
	{
		return CreateEntity_UUIDWithParent(id, _tag, parent);
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
		if(parent == 0) m_rootNode->AddChild(id);
		else
		{
			Entity new_parent = GetEntity(parent);
			if (new_parent)
			{
				new_parent.GetComponent<HierachyComponent>().AddChild(id);
				hi.parent = parent;
			}
			else m_rootNode->AddChild(id);
		}

		return entity;
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		auto it = m_entityMap.find(uuid);
		if (it != m_entityMap.end()) return it->second;
		return Entity();
	}

	template<typename... Component>
	void CopyComponent(Entity from, Entity to)
	{

		([&]() {
			if (from.HasComponent<Component>())
			{
				to.AddOrReplaceComponent<Component>(from.GetComponent<Component>());
			}
			}(), ...);

		
	}

	template<typename... Component>
	void CopyComponent( ComponentGroup<Component...>, Entity from, Entity to)
	{
		CopyComponent<Component...>(from, to);
	}

	template<typename... Component>
	void CopyComponentifExits(Entity from, Entity to)
	{

		([&]() {
			if (from.HasComponent<Component>())
			{
				to.AddOrReplaceComponent<Component>(from.GetComponent<Component>());
			}
			}(), ...);


	}

	template<typename... Component>
	void CopyComponentifExits(ComponentGroup<Component...>, Entity from, Entity to)
	{
		CopyComponentifExits<Component...>(from, to);
	}

	void duplicate_entity_hierachy(Scene* scene, Entity e, Entity parent)
	{
		Entity _res = scene->CreateEntity_UUID(UUID::GenUUID(), e.GetComponent<TagComponent>()._tag);

		CopyComponent(AllComponents{}, e, _res);
		_res.AddOrReplaceComponent<HierachyComponent>();

		scene->SetParent(_res, parent);

		scene->GetScriptRegistry().Iterate(e.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = _res.AddScript(script->GetID());
				*sc_ins = *other;
			});

		HierachyComponent& hierachy = e.GetComponent<HierachyComponent>();
		Scene* _scene = e.GetScene();
		for (auto& i : hierachy.children)
		{
			Entity entity = _scene->GetEntity(i);
			duplicate_entity_hierachy(scene,entity, _res);
		}
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity res = CreateEntity_UUID(UUID::GenUUID(), entity.GetComponent<TagComponent>()._tag);

		CopyComponent(AllComponents{}, entity, res);
		

		
		entity.GetScene()->GetScriptRegistry().Iterate(entity.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = res.AddScript(script->GetID());
				*sc_ins = *other;
			});	


		res.AddOrReplaceComponent<HierachyComponent>();
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();

		if (GetEntity(entity.GetID()) && hi.HasParent())// The entity arguement is a member of the scene
		{
			Entity parent = GetEntity(entity.GetComponent<HierachyComponent>().parent);
			SetParent(res, parent);
		}

		Scene* scene = entity.GetScene();
		for (auto& i : hi.children)
		{
			Entity d_child = scene->GetEntity(i);
			duplicate_entity_hierachy(this, d_child, res);
		}

		return res;
	}

	bool Scene::CopyEntity(Entity entity, Entity src)
	{
		CopyComponentifExits(AllComponents{}, src, entity);



		src.GetScene()->GetScriptRegistry().Iterate(entity.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = entity.AddScript(script->GetID());
				*sc_ins = *other;
			});
		return true;
	}



	void Scene::DestroyEntity(Entity entity)
	{
		if (this != entity.GetScene())
		{
			TRC_WARN("Can't destory an entity that is not a member of a scene, scene name: {}", GetName());
			return;
		}

		// Checking if the entity still exists in the scene
		if (!GetEntity(entity.GetID()))
		{
			TRC_WARN("Entity is not valid, Function: {} ", __FUNCTION__);
			return;
		}

		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		for (UUID& i : hi.children)
		{
			Entity child = GetEntity(i);
			DestroyEntity(child);
		}

		m_scriptRegistry.Erase(entity.GetID());
		entity.RemoveComponent<HierachyComponent>();
		m_entityMap.erase(entity.GetComponent<IDComponent>()._id);
		m_registry.destroy(entity);
	}

	Entity Scene::InstanciatePrefab(Ref<Prefab> prefab)
	{
		Entity handle = PrefabManager::get_instance()->GetScene()->GetEntity(prefab->GetHandle());
		Entity result = DuplicateEntity(handle);
		result.AddComponent<PrefabComponent>(prefab);
		return result;
	}

	Entity Scene::InstanciatePrefab(Ref<Prefab> prefab, Entity parent)
	{
		Entity result = InstanciatePrefab(prefab);
		SetParent(result, parent);
		return result;
	}

	bool Scene::ApplyPrefabChanges(Ref<Prefab> prefab)
	{

		auto prefab_view = m_registry.view<PrefabComponent>();

		for (auto entity : prefab_view)
		{
			auto [pf] = prefab_view.get(entity);
			Entity en(entity, this);

			if (!pf.handle) continue;

			if (pf.handle->GetHandle() == prefab->GetHandle())
			{
				Entity handle = PrefabManager::get_instance()->GetScene()->GetEntity(prefab->GetHandle());
				ApplyPrefabChanges(handle, en);
			}

		}
		return true;
	}

	bool Scene::ApplyPrefabChangesOnSceneLoad()
	{

		auto prefab_view = m_registry.view<PrefabComponent>();

		for (auto entity : prefab_view)
		{
			auto [pf] = prefab_view.get(entity);
			Entity en(entity, this);

			if (!pf.handle) continue;

			ApplyPrefabChanges(pf.handle);

		}

		return true;
	}

	void Scene::SetParent(Entity child, Entity parent)
	{
		if ((child.GetScene() != this) || (parent.GetScene() != this))
		{
			TRC_WARN("One of the entities is not part of the scene, scene name: {}, child: {}, parent: {}", m_name, child.GetComponent<TagComponent>()._tag, parent.GetComponent<TagComponent>()._tag);
			return;
		}

		HierachyComponent& parent_hi = parent.GetComponent<HierachyComponent>();
		HierachyComponent& child_hi = child.GetComponent<HierachyComponent>();
		
		if (child_hi.HasParent())
		{
			Entity old_parent = GetEntity(child_hi.parent);
			old_parent.GetComponent<HierachyComponent>().RemoveChild(child.GetID());
		}
		else
		{
			m_rootNode->RemoveChild(child.GetID());
		}

		parent_hi.AddChild(child.GetID());
		child_hi.parent = parent.GetID();

	}

	bool Scene::IsParent(Entity parent, Entity child)
	{
		HierachyComponent& hi = child.GetComponent<HierachyComponent>();
		UUID pId = hi.parent;

		bool result = false;

		while (pId != 0)
		{
			if (pId == parent.GetID())
			{
				result = true;
				break;
			}

			pId = GetEntity(pId).GetComponent<HierachyComponent>().parent;
		}

		return result;
	}

	void Scene::AddToRoot(Entity entity)
	{
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();

		if (hi.HasParent())
		{
			Entity old_parent = GetEntity(hi.parent);
			old_parent.GetComponent<HierachyComponent>().RemoveChild(entity.GetID());
		}

		if (m_rootNode->HasChild(entity.GetID())) return;

		m_rootNode->AddChild(entity.GetID());
		hi.parent = 0;
	}

	Transform Scene::GetEntityWorldTransform(Entity entity)
	{
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		TransformComponent& pose = entity.GetComponent<TransformComponent>();
		Transform transform = pose._transform;

		if (hi.HasParent())
		{
			Transform parent_transform = GetEntityWorldTransform(GetEntity(hi.parent));
			transform = Transform::CombineTransform(parent_transform, transform);
		}

		return transform;
	}

	Transform Scene::GetEntityGlobalPose(Entity entity)
	{
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();

		Transform transform;
		if (m_running)
		{
			glm::vec3 pos, skew, scale;
			glm::vec4 persp;
			glm::quat rot;

			glm::mat4 pose = hi.transform;

			glm::decompose(pose, scale, rot, pos, skew, persp);
			transform.SetPosition(pos);
			transform.SetRotation(rot);
			transform.SetScale(scale);

			return transform;
		}

		TransformComponent& pose = entity.GetComponent<TransformComponent>();
		transform = pose._transform;

		if (hi.HasParent())
		{
			Transform parent_transform = GetEntityGlobalPose(GetEntity(hi.parent));
			transform = Transform::CombineTransform(parent_transform, transform);
		}

		return transform;
	}

	void Scene::ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback)
	{
		for (UUID& uuid : m_rootNode->children)
		{
			Entity entity = GetEntity(uuid);
			callback(entity, uuid, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback);
		}
	}

	void Scene::ResolveHierachyTransforms()
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

	template<typename... Component>
	void CopyComponent(entt::registry& from, entt::registry& to, std::unordered_map<UUID, entt::entity>& entity_map)
	{
		([&]()
			{
				auto view = from.view<Component>();
				for (auto e : view)
				{
					UUID uuid = from.get<IDComponent>(e)._id;
					auto [comp] = view.get(e);
					entt::entity en = entity_map[uuid];
					to.emplace_or_replace<Component>(en, comp);
				}

			}(), ...);

	}

	template<typename... Component>
	void CopyComponent(ComponentGroup<Component...>, entt::registry& from, entt::registry& to, std::unordered_map<UUID, entt::entity>& entity_map)
	{
		CopyComponent<Component...>(from, to, entity_map);
	}



	void Scene::Copy(Ref<Scene> from, Ref<Scene> to)
	{

		entt::registry& f_reg = from->m_registry;

		to->Destroy();
		to->Create();

		entt::registry& t_reg = to->m_registry;
		to->m_rootNode->children.clear();
		to->m_entityMap.clear();

		std::unordered_map<UUID, entt::entity> entity_map;

		auto id_view = f_reg.view<IDComponent>();
		for (auto e : id_view)
		{
			UUID uuid = id_view.get<IDComponent>(e)._id;
			Entity en = to->CreateEntity_UUID(uuid, "");
			entity_map[uuid] = en;
		}

		CopyComponent(AllComponents{}, f_reg, t_reg, entity_map);
		CopyComponent(ComponentGroup<HierachyComponent>{}, f_reg, t_reg, entity_map);


		ScriptRegistry::Copy(from->m_scriptRegistry, to->m_scriptRegistry);

		to->m_rootNode->children = from->m_rootNode->children;

	}

	void Scene::ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback, bool child_first)
	{
		for (auto& i : hierachy.children)
		{
			Entity entity = GetEntity(i);
			if(!child_first) callback(entity, i, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback);
			if (child_first) callback(entity, i, this);
		}
	}

	void apply_prefab_changes(Scene* scene, Entity e, Entity src)
	{


		HierachyComponent& hi = src.GetComponent<HierachyComponent>();
		HierachyComponent& e_hi = e.GetComponent<HierachyComponent>();

		int i = 0;
		for (auto& j : hi.children)
		{
			Entity _next = src.GetScene()->GetEntity(hi.children[i]);
			Entity _e = e.GetScene()->GetEntity(e_hi.children[i]);
			apply_prefab_changes(scene, _e, _next);
			i++;
		}

		scene->CopyEntity(e, src);

	}

	void Scene::ApplyPrefabChanges(Entity prefab_handle, Entity entity)
	{


		HierachyComponent& hi = prefab_handle.GetComponent<HierachyComponent>();

		HierachyComponent& e_hi = entity.GetComponent<HierachyComponent>();

		int i = 0;
		for (auto& j : hi.children)
		{
			Entity _next = prefab_handle.GetScene()->GetEntity(hi.children[i]);
			Entity _e = entity.GetScene()->GetEntity(e_hi.children[i]);
			apply_prefab_changes(this, _e, _next);
			i++;
		}
		TransformComponent pose = entity.GetComponent<TransformComponent>();
		CopyEntity(entity, prefab_handle);
		entity.AddOrReplaceComponent<TransformComponent>(pose);
	}

	void Scene::OnConstructHierachyComponent(entt::registry& reg, entt::entity ent)
	{
		
	}

	void Scene::OnDestroyHierachyComponent(entt::registry& reg, entt::entity ent)
	{
		Entity entity(ent, this);
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (hi.HasParent())
		{
			Entity parent = GetEntity(hi.parent);
			if (parent)
			{
				parent.GetComponent<HierachyComponent>().RemoveChild(entity.GetID());
			}
		}
		else
		{
			m_rootNode->RemoveChild(entity.GetID());
		}

		//TODO: Allow ProcessEntityHierachy() take in lambda's
		std::function<void(Entity, UUID, Scene*)> destroy_children = [](Entity entity,UUID, Scene* scene)
		{
			scene->DestroyEntity(entity);
		};
		ProcessEntityHierachy(hi, destroy_children, true);
			
	}

	void Scene::OnConstructRigidBodyComponent(entt::registry& reg, entt::entity ent)
	{
	}

	void Scene::OnDestroyRigidBodyComponent(entt::registry& reg, entt::entity ent)
	{
	}

}