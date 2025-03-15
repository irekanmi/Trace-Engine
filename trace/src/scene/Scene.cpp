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
#include "animation/AnimationPose.h"
#include "core/Coretypes.h"
#include "core/Utils.h"

#include "glm/gtx/matrix_decompose.hpp"

namespace trace {
	void Scene::Create()
	{
		m_scriptRegistry.Init(this);
		m_rootNode = new HierachyComponent(); // TODO: Use Custom Allocator

		m_registry.on_construct<HierachyComponent>().connect<&Scene::OnConstructHierachyComponent>(*this);
		m_registry.on_destroy<HierachyComponent>().connect<&Scene::OnDestroyHierachyComponent>(*this);

		m_registry.on_construct<AnimationGraphController>().connect<&Scene::OnConstructAnimationGraphController>(*this);
		m_registry.on_destroy<AnimationGraphController>().connect<&Scene::OnDestroyAnimationGraphController>(*this);

		m_registry.on_construct<SequencePlayer>().connect<&Scene::OnConstructSequencePlayer>(*this);
		m_registry.on_destroy<SequencePlayer>().connect<&Scene::OnDestroySequencePlayer>(*this);

		m_registry.on_construct<RigidBodyComponent>().connect<&Scene::OnConstructRigidBodyComponent>(*this);
		m_registry.on_destroy<RigidBodyComponent>().connect<&Scene::OnDestroyRigidBodyComponent>(*this);

		m_registry.on_construct<BoxColliderComponent>().connect<&Scene::OnConstructBoxColliderComponent>(*this);
		m_registry.on_destroy<BoxColliderComponent>().connect<&Scene::OnDestroyBoxColliderComponent>(*this);

		m_registry.on_construct<SphereColliderComponent>().connect<&Scene::OnConstructSphereColliderComponent>(*this);
		m_registry.on_destroy<SphereColliderComponent>().connect<&Scene::OnDestroySphereColliderComponent>(*this);

		m_registry.on_construct<CharacterControllerComponent>().connect<&Scene::OnConstructCharacterControllerComponent>(*this);
		m_registry.on_destroy<CharacterControllerComponent>().connect<&Scene::OnDestroyCharacterControllerComponent>(*this);

	}
	void Scene::Destroy()
	{
		m_registry.on_construct<RigidBodyComponent>().disconnect<&Scene::OnConstructRigidBodyComponent>(*this);
		m_registry.on_destroy<RigidBodyComponent>().disconnect<&Scene::OnDestroyRigidBodyComponent>(*this);

		m_registry.on_construct<BoxColliderComponent>().disconnect<&Scene::OnConstructBoxColliderComponent>(*this);
		m_registry.on_destroy<BoxColliderComponent>().disconnect<&Scene::OnDestroyBoxColliderComponent>(*this);

		m_registry.on_construct<SphereColliderComponent>().disconnect<&Scene::OnConstructSphereColliderComponent>(*this);
		m_registry.on_destroy<SphereColliderComponent>().disconnect<&Scene::OnDestroySphereColliderComponent>(*this);

		m_registry.on_construct<CharacterControllerComponent>().disconnect<&Scene::OnConstructCharacterControllerComponent>(*this);
		m_registry.on_destroy<CharacterControllerComponent>().disconnect<&Scene::OnDestroyCharacterControllerComponent>(*this);

		m_registry.on_construct<AnimationGraphController>().disconnect<&Scene::OnConstructAnimationGraphController>(*this);
		m_registry.on_destroy<AnimationGraphController>().disconnect<&Scene::OnDestroyAnimationGraphController>(*this);

		m_registry.on_construct<SequencePlayer>().disconnect<&Scene::OnConstructSequencePlayer>(*this);
		m_registry.on_destroy<SequencePlayer>().disconnect<&Scene::OnDestroySequencePlayer>(*this);

		m_registry.on_construct<HierachyComponent>().disconnect<&Scene::OnConstructHierachyComponent>(*this);
		m_registry.on_destroy<HierachyComponent>().disconnect<&Scene::OnDestroyHierachyComponent>(*this);
		delete m_rootNode; // TODO: Use Custom Allocator
		m_registry.clear();
		m_scriptRegistry.Clear();
	}
	void Scene::BeginFrame()
	{
		if (m_running)
		{
			if (!m_entityToDestroy.empty())
			{
				for (Entity& entity : m_entityToDestroy)
				{
					destroy_entity(entity);
				}
				m_entityToDestroy.clear();
			}
		}
	}
	void Scene::EndFrame()
	{
	}
	void Scene::OnStart()
	{
		ResolveHierachyTransforms();

		
		m_running = true;
		auto animations = m_registry.view<AnimationComponent>();
		for (auto i : animations)
		{
			auto [anim_comp] = animations.get(i);
			if (!anim_comp.animation)
			{
				continue;
			}
			Entity entity(i, this);
			anim_comp.InitializeEntities(this);

			if (anim_comp.play_on_start && entity.HasComponent<ActiveComponent>())
			{
				anim_comp.Start();
			}
		}

		auto animation_graphs = m_registry.view<AnimationGraphController>();
		for (auto i : animation_graphs)
		{
			auto [anim_graph] = animation_graphs.get(i);
			Entity entity(i, this);
			if (!anim_graph.graph.GetGraph() || !anim_graph.play_on_start || !entity.HasComponent<ActiveComponent>())
			{
				continue;
			}
			anim_graph.graph.Start(this, entity.GetID());

		}

		auto sequences = m_registry.view<SequencePlayer>();
		for (auto i : sequences)
		{
			auto [seq] = sequences.get(i);

			Entity entity(i, this);
			if (!seq.play_on_start || !entity.HasComponent<ActiveComponent>())
			{
				continue;
			}

			seq.sequence.Start(this, entity.GetID());

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
								if (data.type == ScriptFieldType::String)
								{
									continue;
								}
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
	void Scene::OnPhysicsStart()
	{
		PhysicsFunc::CreateScene3D(m_physics3D, glm::vec3(0.0f, -9.81f, 0.0f));
		if (m_physics3D)
		{
			auto rigid_view = m_registry.view<TransformComponent, RigidBodyComponent>();
			for (auto i : rigid_view)
			{
				auto [pose, rigid] = rigid_view.get(i);
				PhysicsFunc::CreateRigidBody_Scene(m_physics3D, rigid.body, pose._transform);
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
				PhysicsFunc::CreateShapeWithTransform(m_physics3D, box._internal, box.shape, local, box.is_trigger);
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

			auto controllers = m_registry.view<TransformComponent, CharacterControllerComponent>();
			for (auto i : controllers)
			{
				auto [pose, charac] = controllers.get(i);

				Entity entity(i, this);

				float scale_x = pose._transform.GetScale().x;
				float scale_y = pose._transform.GetScale().y;
				float scale_z = pose._transform.GetScale().z;
				charac.character.radius *= (scale_x + scale_z) / 2.0f;//TODO: Determine maybe scale in transform should be used or not
				charac.character.height *= scale_y;//TODO: Determine maybe scale in transform should be used or not

				PhysicsFunc::CreateCharacterController(charac.character, m_physics3D, pose._transform);
				PhysicsFunc::SetControllerDataPtr(charac.character, &m_entityMap[entity.GetID()]);
			}
		}
	}
	void Scene::OnStop()
	{
		auto sequences = m_registry.view<SequencePlayer>();
		for (auto i : sequences)
		{
			auto [seq] = sequences.get(i);

			Entity entity(i, this);
			seq.sequence.Stop(this, entity.GetID());

		}

		auto animations = m_registry.view<AnimationComponent>();
		for (auto i : animations)
		{
			auto [anim_comp] = animations.get(i);
			Entity entity(i, this);
			anim_comp.Stop();
		}

		auto animation_graphs = m_registry.view<AnimationGraphController>();
		for (auto i : animation_graphs)
		{
			auto [anim_graph] = animation_graphs.get(i);
			
			Entity entity(i, this);
			anim_graph.graph.Stop(this, entity.GetID());
		}

		m_entityToDestroy.clear();
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
	void Scene::OnPhysicsStop()
	{
		if (m_physics3D)
		{

			auto controllers = m_registry.view<CharacterControllerComponent>();
			for (auto i : controllers)
			{
				auto [ charac] = controllers.get(i);

				PhysicsFunc::DestroyCharacterController(charac.character, m_physics3D);

			}

			auto bodies = m_registry.view<RigidBodyComponent>();
			for (auto i : bodies)
			{
				auto [rigid] = bodies.get(i);

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
	void Scene::OnUpdate(float deltaTime)
	{
		ResolveHierachyTransforms();

	}

	void Scene::OnScriptUpdate(float deltaTime)
	{
		m_scriptRegistry.Iterate([deltaTime, this](ScriptRegistry::ScriptManager& manager)
			{
				ScriptMethod* on_update = manager.script->GetMethod("OnUpdate");

				if (!on_update) return;
				float dt = deltaTime;

				for (auto& i : manager.entities)
				{
					Entity entity = this->GetEntity(i);
					if (!entity.HasComponent<ActiveComponent>())//TODO: Optimize by creating a script component to use to filter inactive entities get<ScriptComponent, ActiveComponent>();
					{
						continue;
					}
					ScriptInstance& ins = manager.instances[manager.handle_map[i]];
					void* params[1] =
					{
						&dt
					};
					InvokeScriptMethod_Instance(*on_update, ins, params);
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

			auto controllers = m_registry.view<TransformComponent, CharacterControllerComponent>();
			for (auto i : controllers)
			{
				auto [pose, charac] = controllers.get(i);

				Entity entity(i, this);

				glm::vec3 curr_pos(0.0f);
				PhysicsFunc::GetCharacterControllerPosition(charac.character, curr_pos);

				pose._transform.SetPosition(curr_pos);
			}
		}
	}

	void Scene::OnAnimationUpdate(float deltaTime)
	{
		auto animations = m_registry.view<AnimationComponent, ActiveComponent>();
		for (auto i : animations)
		{
			Entity entity(i, this);
			auto [anim_comp, active] = animations.get(i);
			if (!anim_comp.animation)
			{
				continue;
			}
			anim_comp.Update(deltaTime, this);
		}

		auto animation_graphs = m_registry.view<AnimationGraphController, ActiveComponent>();
		for (auto i : animation_graphs)
		{
			auto [anim_graph, active] = animation_graphs.get(i);
			if (!anim_graph.graph.GetGraph())
			{
				continue;
			}

			if (!anim_graph.graph.HasStarted())
			{
				continue;
			}

			Entity entity(i, this);

			anim_graph.graph.Update(deltaTime, this, entity.GetID());
		}

		auto sequences = m_registry.view<SequencePlayer, ActiveComponent>();
		for (auto i : sequences)
		{
			auto [seq, active] = sequences.get(i);
			if (!seq.sequence.HasStarted())
			{
				continue;
			}
			Entity entity(i, this);
			seq.sequence.Update(this, deltaTime);

		}

	}


	void Scene::OnRender()
	{
		Camera* main_camera = nullptr;

		auto view = m_registry.view<CameraComponent , TransformComponent, ActiveComponent>();
	
		for (auto entity : view)
		{
			auto [camera, _transform, active] = view.get(entity);
			if (camera.is_main)
			{
				Entity obj(entity, this);
				Transform final_transform = GetEntityGlobalPose(obj, true);
				camera._camera.SetPosition(final_transform.GetPosition());
				glm::vec3 forward = final_transform.GetForward();
				glm::vec3 up = final_transform.GetUp();
				glm::vec3 right = final_transform.GetRight();
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
		//TODO: Optimize resolve hierachy transform by only calculating it when needed and save it for later use during that frame

		Renderer* renderer = Renderer::get_instance();



		auto sun_light_group = m_registry.view<SunLight, HierachyComponent, ActiveComponent>();

		for (auto entity : sun_light_group)
		{
			Entity object(entity, this);
			auto [light, transform, active] = sun_light_group.get(entity);
			Transform final_transform = GetEntityWorldTransform(object);

			Light light_data = {};
			light_data.position = glm::vec4(final_transform.GetPosition(), 0.0f);
			light_data.direction = glm::vec4(final_transform.GetForward(), 0.0f);
			light_data.color = glm::vec4(light.color, 0.0f);
			light_data.params1 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			light_data.params2 = glm::vec4(0.0f, light.intensity, light.cast_shadows ? 1.0f : 0.0f, 0.0f);


			renderer->AddLight(cmd_list, light_data, LightType::DIRECTIONAL);
		}

		auto spot_light_group = m_registry.view<SpotLight, HierachyComponent, ActiveComponent>();

		for (auto entity : spot_light_group)
		{
			Entity object(entity, this);
			auto [light, transform, active] = spot_light_group.get(entity);
			Transform final_transform = GetEntityWorldTransform(object);

			Light light_data = {};
			light_data.position = glm::vec4(final_transform.GetPosition(), 0.0f);
			light_data.direction = glm::vec4(final_transform.GetForward(), 0.0f);
			light_data.color = glm::vec4(light.color, 0.0f);
			light_data.params1 = glm::vec4(0.0f, 0.0f, 0.0f, light.innerCutOff);
			light_data.params2 = glm::vec4(light.outerCutOff, light.intensity, light.cast_shadows ? 1.0f : 0.0f, 0.0f);


			renderer->AddLight(cmd_list, light_data, LightType::SPOT);
		}



		auto point_light_group = m_registry.view<PointLight, HierachyComponent, ActiveComponent>();

		for (auto entity : point_light_group)
		{
			Entity object(entity, this);
			auto [light, transform, active] = point_light_group.get(entity);
			Transform final_transform = GetEntityWorldTransform(object);
			
			Light light_data = {};
			light_data.position = glm::vec4(final_transform.GetPosition(), 0.0f);
			light_data.direction = glm::vec4(final_transform.GetForward(), 0.0f);
			light_data.color = glm::vec4(light.color, 0.0f);
			light_data.params1 = glm::vec4( light.constant, light.linear, light.quadratic, 0.0f);
			light_data.params2 = glm::vec4(0.0f, light.intensity, light.cast_shadows ? 1.0f : 0.0f, 0.0f);


			renderer->AddLight(cmd_list, light_data, LightType::POINT);
		}



		auto group = m_registry.group<MeshComponent, HierachyComponent, ActiveComponent>();

		for (auto entity : group)
		{
			auto [mesh, transform, active] = group.get(entity);

			renderer->DrawMesh(cmd_list, mesh._mesh, transform.transform); // TODO Implement Hierachies

		}

		auto model_view = m_registry.view<ModelComponent, ModelRendererComponent, HierachyComponent, ActiveComponent>();

		for (auto entity : model_view)
		{
			auto [model, model_renderer, transform, active] = model_view.get(entity);

			renderer->DrawModel(cmd_list, model._model, model_renderer._material, transform.transform, model_renderer.cast_shadow); // TODO Implement Hierachies

		}

		auto skinned_model_view = m_registry.view<SkinnedModelRenderer, HierachyComponent, ActiveComponent>();

		for (auto entity : skinned_model_view)
		{
			Entity obj(entity, this);
			auto [model_renderer, transform, active] = skinned_model_view.get(entity);

			if (!model_renderer._material || !model_renderer._model || !model_renderer.GetSkeleton())
			{
				continue;
			}
			model_renderer.runtime_skeleton.GetGlobalPose(model_renderer.bone_transforms, obj.GetID());
			renderer->DrawSkinnedModel(cmd_list, model_renderer._model, model_renderer._material, transform.transform, model_renderer.bone_transforms.data(), static_cast<uint32_t>(model_renderer.bone_transforms.size()), model_renderer.cast_shadow);


		}

		auto text_view = m_registry.view<TextComponent, HierachyComponent, ActiveComponent>();

		for (auto entity : text_view)
		{
			auto [txt, transform, active] = text_view.get(entity);

			glm::vec3 color = txt.color * txt.intensity;
			renderer->DrawString(cmd_list, txt.font, txt.text, color, transform.transform); // TODO Implement Hierachies

		}

		auto images_view = m_registry.view<ImageComponent, HierachyComponent, ActiveComponent>();

		for (auto entity : images_view)
		{
			auto [img, transform, active] = images_view.get(entity);

			if (img.image)
			{
				renderer->DrawImage(cmd_list, img.image, transform.transform, img.color); // TODO Implement Hierachies
			}



		}

	}

	void Scene::OnViewportChange(float width, float height)
	{
		auto view = m_registry.view<CameraComponent>();

		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camera._camera.SetScreenWidth(width);
			camera._camera.SetScreenHeight(height);
		}
	}

	bool Scene::InitializeSceneComponents()
	{
		

		for (UUID& id : m_rootNode->children)
		{
			Entity entity = GetEntity(id);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			if (hi.is_enabled)
			{
				EnableEntity(entity);
			}
		}

		auto animation_graphs = m_registry.view<AnimationGraphController>();
		for (auto i : animation_graphs)
		{
			auto [anim_graph] = animation_graphs.get(i);
			Entity entity(i, this);

			anim_graph.graph.CreateInstance(anim_graph.graph.GetGraph(), this, entity.GetID());

		}

		auto skinned_models = m_registry.view<SkinnedModelRenderer>();
		for (auto i : skinned_models)
		{
			auto [model] = skinned_models.get(i);
			Entity entity(i, this);

			model.SetSkeleton(model.runtime_skeleton.GetSkeleton(), this, entity.GetID());

		}

		auto sequences = m_registry.view<SequencePlayer>();
		for (auto i : sequences)
		{
			auto [seq] = sequences.get(i);

			Entity entity(i, this);
			
			seq.sequence.CreateInstance(seq.sequence.GetSequence(), this);

		}

		return true;
	}

	void Scene::EnableEntity(Entity entity)
	{
		if (entity.HasComponent<ActiveComponent>())
		{
			return;
		}

		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		hi.is_enabled = true;
		entity.AddComponent<ActiveComponent>();
		for (auto& child_id : hi.children)
		{
			Entity child = GetEntity(child_id);
			enable_child_entity(child);
		}
		m_scriptRegistry.Iterate(entity.GetID(), [](UUID uuid, Script* script, ScriptInstance* instance)
			{
				ScriptMethod* on_enable = script->GetMethod("OnEnable");
				if (!on_enable)
				{
					return;
				}


				InvokeScriptMethod_Instance(*on_enable, *instance, nullptr);
			});
		
	}

	void Scene::DisableEntity(Entity entity)
	{
		if (entity.HasComponent<ActiveComponent>())
		{
			entity.RemoveComponent<ActiveComponent>();
		}

		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		hi.is_enabled = false;
		for (auto& child_id : hi.children)
		{
			Entity child = GetEntity(child_id);
			disable_child_entity(child);
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
		tag.SetTag(_tag.empty() ? "New Entity" : _tag);
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
		if (it != m_entityMap.end())
		{
			return it->second;
		}

		return Entity();
	}

	Entity Scene::GetEntityByName(const std::string& name)
	{
		return GetEntityByName(STR_ID(name));
	}

	Entity Scene::GetEntityByName(StringID name)
	{
		auto tag_view = m_registry.view<TagComponent>();

		for (auto entity : tag_view)
		{
			auto [tag] = tag_view.get(entity);

			if (tag.GetStringID() == name)
			{
				Entity en(entity, this);
				return en;
			}

		}

		return Entity();
	}

	Entity Scene::GetChildEntityByName(Entity parent, const std::string& name)
	{
		return GetChildEntityByName(parent, STR_ID(name));
	}

	Entity Scene::GetChildEntityByName(Entity parent, StringID name)
	{
		for (UUID& child_id : parent.GetComponent<HierachyComponent>().children)
		{
			Entity child = GetEntity(child_id);
			TagComponent& tag = child.GetComponent<TagComponent>();

			if (tag.GetStringID() == name)
			{
				return child;
			}

			Entity result = GetChildEntityByName(child, name);

			if (result)
			{
				return result;
			}
		}

		return Entity();
	}

	Entity Scene::GetParentByName(Entity entity, std::string parent_name)
	{
		return GetParentByName(entity, STR_ID(parent_name));
	}

	Entity Scene::GetParentByName(Entity entity, StringID parent_name)
	{
		Entity parent = entity.GetParent();

		Entity result;
		while (parent)
		{
			if (parent.GetComponent<TagComponent>().GetStringID() == parent_name)
			{
				result = parent;
				break;
			}

			parent = parent.GetParent();
		}
		return result;
	}

	Entity Scene::FindEnityInHierachy(Entity entity, StringID name)
	{
		Entity parent = entity.GetParent();

		Entity result;
		while (parent)
		{
			result = GetChildEntityByName(parent, name);

			if (result)
			{
				break;
			}

			parent = parent.GetParent();
		}
		return result;
	}

	Entity Scene::GetParentWithAnimation(Entity entity)
	{
		Entity parent = entity.GetParent();

		Entity anim_obj;
		while (parent)
		{
			if (parent.HasComponent<AnimationComponent>())
			{
				anim_obj = parent;
				break;
			}

			parent = parent.GetParent();
		}
		return anim_obj;
	}

	template<typename... Component>
	void CopyComponent(Entity from, Entity to)
	{

		([&]() {
			if (from.HasComponent<Component>() && !to.HasComponent<Component>())
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
		Entity _res = scene->CreateEntity_UUID(UUID::GenUUID(), e.GetComponent<TagComponent>().GetTag());

		_res.RemoveComponent<TransformComponent>();
		CopyComponent(AllComponents{}, e, _res);
		_res.AddOrReplaceComponent<HierachyComponent>();
		scene->SetParent(_res, parent);

		if (e.HasComponent<SkinnedModelRenderer>())
		{
			SkinnedModelRenderer& obj_renderer = _res.AddOrReplaceComponent<SkinnedModelRenderer>();
			SkinnedModelRenderer& skinned_renderer = e.GetComponent<SkinnedModelRenderer>();

			obj_renderer._model = skinned_renderer._model;
			obj_renderer._material = skinned_renderer._material;
			obj_renderer.cast_shadow = skinned_renderer.cast_shadow;
			obj_renderer.SetSkeleton(skinned_renderer.GetSkeleton(), scene, _res.GetID());
		}


		scene->GetScriptRegistry().Iterate(e.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = _res.AddScript(script->GetID());
				*sc_ins = *other;

				if (scene->IsRunning())
				{
					ScriptMethod* constructor = ScriptEngine::get_instance()->GetConstructor();
					CreateScriptInstance(*script, *sc_ins);

					// Setting values ..............
					for (auto& [name, data] : other->GetFields())
					{
						if (data->field_type == ScriptFieldType::String)
						{
							continue;
						}
						char field_data[16];
						other->GetFieldValueInternal(name, field_data, 16);
						sc_ins->SetFieldValueInternal(name, field_data, 16);
					}
					// ..................................................................

					UUID id = _res.GetID();
					void* params[1] =
					{
						&id
					};
					InvokeScriptMethod_Instance(*constructor, *sc_ins, params);
				}
			});

		HierachyComponent& hierachy = e.GetComponent<HierachyComponent>();
		Scene* _scene = e.GetScene();
		for (auto& i : hierachy.children)
		{
			Entity entity = _scene->GetEntity(i);
			duplicate_entity_hierachy(scene,entity, _res);
		}
	}

	//TODO: Update this function to initialize components
	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity res = CreateEntity_UUID(UUID::GenUUID(), entity.GetComponent<TagComponent>().GetTag());		

		res.RemoveComponent<TransformComponent>();
		duplicate_entity(entity, res);

		return res;
	}

	bool Scene::CopyEntity(Entity entity, Entity src)
	{
		CopyComponentifExits(AllComponents{}, src, entity);

		if (src.HasComponent<SkinnedModelRenderer>())
		{
			SkinnedModelRenderer& obj_renderer = entity.AddOrReplaceComponent<SkinnedModelRenderer>();
			SkinnedModelRenderer& skinned_renderer = src.GetComponent<SkinnedModelRenderer>();

			obj_renderer._model = skinned_renderer._model;
			obj_renderer._material = skinned_renderer._material;
			obj_renderer.cast_shadow = skinned_renderer.cast_shadow;
			obj_renderer.SetSkeleton(skinned_renderer.GetSkeleton(), this, entity.GetID());
		}

		src.GetScene()->GetScriptRegistry().Iterate(entity.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = entity.AddScript(script->GetID());
				*sc_ins = *other;
			});



		return true;
	}



	void Scene::DestroyEntity(Entity entity)
	{
		if (m_running)
		{
			m_entityToDestroy.push_back(entity);
		}
		else
		{
			destroy_entity(entity);
		}
	}

	Entity Scene::InstanciatePrefab(Ref<Prefab> prefab)
	{
		Entity handle = PrefabManager::get_instance()->GetScene()->GetEntity(prefab->GetHandle());
		Entity result = DuplicateEntity(handle);
		result.AddComponent<PrefabComponent>(prefab);
		EnableEntity(result);
		return result;
	}

	Entity Scene::InstanciatePrefab(Ref<Prefab> prefab, Entity parent)
	{
		Entity handle = PrefabManager::get_instance()->GetScene()->GetEntity(prefab->GetHandle());
		Entity result = DuplicateEntity(handle);
		result.AddComponent<PrefabComponent>(prefab);
		SetParent(result, parent);
		if (parent.HasComponent<ActiveComponent>())
		{
			EnableEntity(result);
		}
		return result;
	}



	Entity Scene::InstanciateEntity(Entity source, glm::vec3 position)
	{
		if (!m_running)
		{
			return Entity();
		}

		Entity result = CreateEntity_UUID(UUID::GenUUID(), source.GetComponent<TagComponent>().GetTag());

		TransformComponent& pose = result.AddOrReplaceComponent<TransformComponent>(source.GetComponent<TransformComponent>());
		pose._transform.SetPosition(position);
		
		duplicate_entity(source, result);

		EnableEntity(result);

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
			TRC_WARN("One of the entities is not part of the scene, scene name: {}, child: {}, parent: {}", m_name, child.GetComponent<TagComponent>().GetTag(), parent.GetComponent<TagComponent>().GetTag());
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

	Transform Scene::GetEntityGlobalPose(Entity entity, bool recompute)
	{
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();

		Transform transform;
		if (m_running && !recompute)
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

	glm::vec3 Scene::GetEntityWorldPosition(Entity entity)
	{
		if (!entity)
		{
			return glm::vec3(0.0f);
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (!hi.HasParent())
		{
			return entity.GetComponent<TransformComponent>()._transform.GetPosition();
		}
		return glm::vec3(hi.transform[3]);
	}

	glm::quat Scene::GetEntityWorldRotation(Entity entity)
	{
		if (!entity)
		{
			return glm::quat();
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (!hi.HasParent())
		{
			return entity.GetComponent<TransformComponent>()._transform.GetRotation();
		}
		return glm::quat_cast(hi.transform);
	}

	glm::vec3 Scene::GetEntityWorldScale(Entity entity)
	{
		if (!entity)
		{
			return glm::vec3(0.0f);
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (!hi.HasParent())
		{
			return entity.GetComponent<TransformComponent>()._transform.GetScale();
		}
		return glm::vec3(glm::length(hi.transform[0]), glm::length(hi.transform[1]), glm::length(hi.transform[2]));
	}

	void Scene::SetEntityWorldPosition(Entity entity, glm::vec3 position)
	{
		if (!entity)
		{
			return;
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		TransformComponent& pose = entity.GetComponent<TransformComponent>();
		if (!hi.HasParent())
		{
			pose._transform.SetPosition(position);
			return;
		}
		Entity parent = GetEntity(hi.parent);
		HierachyComponent& parent_hi = parent.GetComponent<HierachyComponent>();
		glm::vec3 local_position = glm::vec3(glm::inverse(parent_hi.transform) * glm::vec4(position, 1.0f));
		pose._transform.SetPosition(local_position);

	}

	void Scene::SetEntityWorldRotation(Entity entity, glm::quat rotation)
	{
		if (!entity)
		{
			return;
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		TransformComponent& pose = entity.GetComponent<TransformComponent>();
		if (!hi.HasParent())
		{
			pose._transform.SetRotation(rotation);
			return;
		}
		Entity parent = GetEntity(hi.parent);
		HierachyComponent& parent_hi = parent.GetComponent<HierachyComponent>();
		glm::quat local_rotation = glm::inverse(glm::quat_cast(parent_hi.transform)) * rotation;
		pose._transform.SetRotation(local_rotation);
	}

	void Scene::SetEntityWorldScale(Entity entity, glm::vec3 scale)
	{
		if (!entity)
		{
			return;
		}
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		TransformComponent& pose = entity.GetComponent<TransformComponent>();
		if (!hi.HasParent())
		{
			pose._transform.SetScale(scale);
			return;
		}
		Entity parent = GetEntity(hi.parent);
		HierachyComponent& parent_hi = parent.GetComponent<HierachyComponent>();
		glm::vec3 parent_world_scale = glm::vec3(
			glm::length(parent_hi.transform[0]),
			glm::length(parent_hi.transform[1]),
			glm::length(parent_hi.transform[2])
		);
		glm::vec3 local_scale = scale / parent_world_scale;
		pose._transform.SetPosition(local_scale);
	}

	void Scene::ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback, bool skip_inactive)
	{
		for (UUID& uuid : m_rootNode->children)
		{
			Entity entity = GetEntity(uuid);
			if (!entity.HasComponent<ActiveComponent>() && skip_inactive)
			{
				continue;
			}
			callback(entity, uuid, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback, skip_inactive);
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

		auto skin_view = f_reg.view<SkinnedModelRenderer>();
		for (auto e : skin_view)
		{
			Entity from_entity = Entity(e, from.get());
			SkinnedModelRenderer& from_model = skin_view.get<SkinnedModelRenderer>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			SkinnedModelRenderer& to_model = to_entity.GetComponent<SkinnedModelRenderer>();
			
			to_model.SetSkeleton(from_model.GetSkeleton(), to.get(), to_entity.GetID());

		}

		/*auto graph_view = f_reg.view<AnimationGraphController>();
		for (auto e : graph_view)
		{
			Entity from_entity = Entity(e, from.get());
			AnimationGraphController& from_model = graph_view.get<AnimationGraphController>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			to_entity.RemoveComponent<AnimationGraphController>();
			to_entity.AddOrReplaceComponent<AnimationGraphController>(from_model);			

		}*/

		auto sequence_view = f_reg.view<SequencePlayer>();
		for (auto e : sequence_view)
		{
			Entity from_entity = Entity(e, from.get());
			SequencePlayer& from_model = sequence_view.get<SequencePlayer>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			to_entity.RemoveComponent<SequencePlayer>();
			to_entity.AddOrReplaceComponent<SequencePlayer>(from_model);

		}


		ScriptRegistry::Copy(from->m_scriptRegistry, to->m_scriptRegistry);

		to->m_rootNode->children = from->m_rootNode->children;

		to->InitializeSceneComponents();

	}

	void Scene::ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback, bool skip_inactive, bool child_first)
	{
		for (auto& i : hierachy.children)
		{
			Entity entity = GetEntity(i);
			if (!entity.HasComponent<ActiveComponent>() && skip_inactive)
			{
				continue;
			}
			if(!child_first) callback(entity, i, this);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			ProcessEntityHierachy(hi, callback, skip_inactive);
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
		//Entity entity(ent, this);
		//HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		//if (hi.HasParent())
		//{
		//	Entity parent = GetEntity(hi.parent);
		//	if (parent)
		//	{
		//		parent.GetComponent<HierachyComponent>().RemoveChild(entity.GetID());
		//	}
		//}
		//else
		//{
		//	m_rootNode->RemoveChild(entity.GetID());
		//}

		////TODO: Allow ProcessEntityHierachy() take in lambda's
		//std::function<void(Entity, UUID, Scene*)> destroy_children = [](Entity entity,UUID, Scene* scene)
		//{
		//	scene->DestroyEntity(entity);
		//};
		//ProcessEntityHierachy(hi, destroy_children, true);
			
	}

	void Scene::OnConstructRigidBodyComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			TransformComponent& pose = entity.GetComponent<TransformComponent>();
			RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
			void*& internal_ptr = rigid.body.GetInternal();
			internal_ptr = nullptr;//NOTE: Because we are trying to copy from another object
			PhysicsFunc::CreateRigidBody_Scene(m_physics3D, rigid.body, pose._transform);
		}
	}
	void Scene::OnDestroyRigidBodyComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
			PhysicsFunc::RemoveActor(m_physics3D, rigid.body.GetInternal());
			PhysicsFunc::DestroyRigidBody(rigid.body);
		}
	}


	void Scene::OnConstructBoxColliderComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			TransformComponent& pose = entity.GetComponent<TransformComponent>();
			BoxColliderComponent& box = entity.GetComponent<BoxColliderComponent>();

			glm::vec3 extent = box.shape.box.half_extents;
			box.shape.box.half_extents *= pose._transform.GetScale();
			Transform local;
			local.SetPosition(pose._transform.GetPosition() + box.shape.offset);
			local.SetRotation(pose._transform.GetRotation());
			box._internal = nullptr;//NOTE: Because we are trying to copy from another object
			PhysicsFunc::CreateShapeWithTransform(m_physics3D, box._internal, box.shape, local, box.is_trigger);
			//Temp _______________
			PhysicsFunc::SetShapeMask(box._internal, BIT(1), BIT(1));
			// -------------------

			box.shape.box.half_extents = extent;


			UUID _id = entity.GetID();
			Entity* shp_ptr = &m_entityMap[_id];
			PhysicsFunc::SetShapePtr(box._internal, shp_ptr);
			if (!box.is_trigger && entity.HasComponent<RigidBodyComponent>())
			{
				RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
				TRC_ASSERT(rigid.body.GetInternal() != nullptr, "Invalid Rigid body handle, Function: {}", __FUNCTION__);
				PhysicsFunc::SetRigidBodyTransform(rigid.body, local);
				PhysicsFunc::AttachShape(box._internal, rigid.body.GetInternal());

			}
		}
	}

	void Scene::OnDestroyBoxColliderComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			BoxColliderComponent& box = entity.GetComponent<BoxColliderComponent>();

			PhysicsFunc::DestroyShape(box._internal);
		}
	}

	void Scene::OnConstructSphereColliderComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			TransformComponent& pose = entity.GetComponent<TransformComponent>();
			SphereColliderComponent& sc = entity.GetComponent<SphereColliderComponent>();

			float radius = sc.shape.sphere.radius;
			sc.shape.sphere.radius *= pose._transform.GetScale().x; //TODO: Determine maybe scale in transform should be used or not
			Transform local;
			local.SetPosition(pose._transform.GetPosition() + sc.shape.offset);
			local.SetRotation(pose._transform.GetRotation());
			sc._internal = nullptr;//NOTE: Because we are trying to copy from another object
			PhysicsFunc::CreateShapeWithTransform(m_physics3D, sc._internal, sc.shape, local, sc.is_trigger);
			sc.shape.sphere.radius = radius;
			//Temp _______________
			PhysicsFunc::SetShapeMask(sc._internal, BIT(1), BIT(1));
			// -------------------

			UUID _id = entity.GetID();
			Entity* shp_ptr = &m_entityMap[_id];
			PhysicsFunc::SetShapePtr(sc._internal, shp_ptr);
			if (!sc.is_trigger && entity.HasComponent<RigidBodyComponent>())
			{
				RigidBodyComponent& rigid = entity.GetComponent<RigidBodyComponent>();
				TRC_ASSERT(rigid.body.GetInternal() != nullptr, "Invalid Rigid body handle, Function: {}", __FUNCTION__);
				PhysicsFunc::SetRigidBodyTransform(rigid.body, local);
				PhysicsFunc::AttachShape(sc._internal, rigid.body.GetInternal());

			}

		}
	}

	void Scene::OnDestroySphereColliderComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			SphereColliderComponent& sc = entity.GetComponent<SphereColliderComponent>();
			PhysicsFunc::DestroyShape(sc._internal);
		}

	}

	void Scene::OnConstructCharacterControllerComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			TransformComponent& pose = entity.GetComponent<TransformComponent>();
			CharacterControllerComponent& charac = entity.GetComponent<CharacterControllerComponent>();

			float scale_x = pose._transform.GetScale().x;
			float scale_y = pose._transform.GetScale().y;
			float scale_z = pose._transform.GetScale().z;
			charac.character.radius *= (scale_x + scale_z) / 2.0f;//TODO: Determine maybe scale in transform should be used or not
			charac.character.height *= scale_y;//TODO: Determine maybe scale in transform should be used or not

			PhysicsFunc::CreateCharacterController(charac.character, m_physics3D, pose._transform);
			PhysicsFunc::SetControllerDataPtr(charac.character, &m_entityMap[entity.GetID()]);
		}
	}

	void Scene::OnDestroyCharacterControllerComponent(entt::registry& reg, entt::entity ent)
	{
		if (m_running)
		{
			Entity entity(ent, this);
			CharacterControllerComponent& charac = entity.GetComponent<CharacterControllerComponent>();

			PhysicsFunc::DestroyCharacterController(charac.character, m_physics3D);
		}
	}

	void Scene::OnConstructAnimationGraphController(entt::registry& reg, entt::entity ent)
	{
		/*Entity entity(ent, this);
		AnimationGraphController& controller = entity.GetComponent<AnimationGraphController>();
		controller.graph.DestroyInstance();
		controller.graph.CreateInstance(controller.graph.GetGraph(), this, entity.GetID());*/
	}

	void Scene::OnDestroyAnimationGraphController(entt::registry& reg, entt::entity ent)
	{
		/*Entity entity(ent, this);
		AnimationGraphController& controller = entity.GetComponent<AnimationGraphController>();
		controller.graph.DestroyInstance();*/
	}

	void Scene::OnConstructSequencePlayer(entt::registry& reg, entt::entity ent)
	{
		Entity entity(ent, this);
		SequencePlayer& player = entity.GetComponent<SequencePlayer>();
		player.sequence.CreateInstance(player.sequence.GetSequence(), this);
	}

	void Scene::OnDestroySequencePlayer(entt::registry& reg, entt::entity ent)
	{
		Entity entity(ent, this);
		SequencePlayer& player = entity.GetComponent<SequencePlayer>();
		player.sequence.DestroyInstance();
	}

	void Scene::enable_child_entity(Entity entity)
	{
		if (entity.HasComponent<ActiveComponent>())
		{
			return;
		}

		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (!hi.is_enabled)
		{
			return;// Don't enable entity if it has been manuallay disabled
		}
		entity.AddComponent<ActiveComponent>();
		for (auto& child_id : hi.children)
		{
			Entity child = GetEntity(child_id);
			enable_child_entity(child);
		}
		m_scriptRegistry.Iterate(entity.GetID(), [](UUID uuid, Script* script, ScriptInstance* instance)
			{
				ScriptMethod* on_enable = script->GetMethod("OnEnable");
				if (!on_enable)
				{
					return;
				}


				InvokeScriptMethod_Instance(*on_enable, *instance, nullptr);
			});
	}

	void Scene::disable_child_entity(Entity entity)
	{
		if (entity.HasComponent<ActiveComponent>())
		{
			entity.RemoveComponent<ActiveComponent>();
		}

		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		for (auto& child_id : hi.children)
		{
			Entity child = GetEntity(child_id);
			disable_child_entity(child);
		}
	}

	void Scene::destroy_entity(Entity entity)
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
		while (hi.children.size() > 0)
		{
			UUID& id = hi.children.front();
			Entity child = GetEntity(id);
			destroy_entity(child);
		}

		if (hi.HasParent())
		{
			Entity parent = GetEntity(hi.parent);
			HierachyComponent& parent_hi = parent.GetComponent<HierachyComponent>();
			parent_hi.RemoveChild(entity.GetID());
		}
		else
		{
			m_rootNode->RemoveChild(entity.GetID());
		}

		if (m_running)
		{
			ScriptEngine::get_instance()->RemoveEnityActionClass(entity.GetID());
		}

		m_scriptRegistry.Erase(entity.GetID());
		m_entityMap.erase(entity.GetID());
		m_registry.destroy(entity);
	}

	void Scene::duplicate_entity(Entity entity, Entity res)
	{
		CopyComponent(AllComponents{}, entity, res);
		res.AddOrReplaceComponent<HierachyComponent>();
		HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
		if (GetEntity(entity.GetID()) && hi.HasParent())// The entity arguement is a member of the scene
		{
			Entity parent = GetEntity(entity.GetComponent<HierachyComponent>().parent);
			SetParent(res, parent);
		}


		entity.GetScene()->GetScriptRegistry().Iterate(entity.GetID(), [&](UUID, Script* script, ScriptInstance* other)
			{
				ScriptInstance* sc_ins = res.AddScript(script->GetID());
				*sc_ins = *other;

				if (m_running)
				{
					ScriptMethod* constructor = ScriptEngine::get_instance()->GetConstructor();
					CreateScriptInstance(*script, *sc_ins);

					// Setting values ..............
					for (auto& [name, data] : other->GetFields())
					{
						if (data->field_type == ScriptFieldType::String)
						{
							continue;
						}
						char field_data[16];
						other->GetFieldValueInternal(name, field_data, 16);
						sc_ins->SetFieldValueInternal(name, field_data, 16);
					}
					// ..................................................................

					UUID id = res.GetID();
					void* params[1] =
					{
						&id
					};
					InvokeScriptMethod_Instance(*constructor, *sc_ins, params);
				}

			});




		if (entity.HasComponent<SkinnedModelRenderer>())
		{
			SkinnedModelRenderer& obj_renderer = res.AddOrReplaceComponent<SkinnedModelRenderer>();
			SkinnedModelRenderer& skinned_renderer = entity.GetComponent<SkinnedModelRenderer>();

			obj_renderer._model = skinned_renderer._model;
			obj_renderer._material = skinned_renderer._material;
			obj_renderer.cast_shadow = skinned_renderer.cast_shadow;
			obj_renderer.SetSkeleton(skinned_renderer.GetSkeleton(), this, res.GetID());
		}



		Scene* scene = entity.GetScene();
		for (auto& i : hi.children)
		{
			Entity d_child = scene->GetEntity(i);
			duplicate_entity_hierachy(this, d_child, res);
		}
	}

	void Scene::OnConstructSkinnedModelRendererComponent(entt::registry& reg, entt::entity ent)
	{
	}

	void Scene::OnDestroySkinnedModelRendererComponent(entt::registry& reg, entt::entity ent)
	{
	}

}