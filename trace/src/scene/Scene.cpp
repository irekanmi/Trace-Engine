#include "pch.h"

#include "Scene.h"
#include "Entity.h"
#include "Componets.h"
#include "render/Renderer.h"

namespace trace {
	void Scene::OnCreate()
	{
	}
	void Scene::OnDestroy()
	{
		m_registry.clear();
	}
	void Scene::OnUpdate(float deltaTime)
	{

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
			if (light._mesh.is_valid())
				renderer->DrawLight(cmd_list, light._mesh, light._light, light.light_type);
			else
				renderer->AddLight(cmd_list, light._light, light.light_type);

		}

		auto group = m_registry.group<MeshComponent, TransformComponent>();

		for (auto entity : group)
		{
			auto [mesh, transform] = group.get(entity);

			renderer->DrawMesh(cmd_list, mesh._mesh, transform._transform.GetLocalMatrix()); // TODO Implement Hierachies

		}

		auto model_view = m_registry.view<ModelComponent, ModelRendererComponent, TransformComponent>();

		for (auto entity : model_view)
		{
			auto [model, model_renderer, transform] = model_view.get(entity);

			renderer->DrawModel(cmd_list, model._model, model_renderer._material, transform._transform.GetLocalMatrix()); // TODO Implement Hierachies

		}

		auto text_view = m_registry.view<TextComponent, TransformComponent>();

		for (auto entity : text_view)
		{
			auto [txt, transform] = text_view.get(entity);

			renderer->DrawString(cmd_list, txt.font, txt.text, transform._transform.GetLocalMatrix()); // TODO Implement Hierachies

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
		entt::entity handle = m_registry.create();
		Entity entity(handle, this);
		TagComponent& tag = entity.AddComponent<TagComponent>();
		tag._tag = _tag.empty() ? "New Entity" : _tag;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<IDComponent>()._id = id;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_registry.destroy(entity);
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
		CopyComponent<TransformComponent>(f_reg, t_reg, entity_map);
		CopyComponent<CameraComponent>(f_reg, t_reg, entity_map);
		CopyComponent<LightComponent>(f_reg, t_reg, entity_map);
		CopyComponent<MeshComponent>(f_reg, t_reg, entity_map);
		CopyComponent<ModelComponent>(f_reg, t_reg, entity_map);

	}

}