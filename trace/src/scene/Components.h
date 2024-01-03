#pragma once

#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"
#include "render/Mesh.h"
#include "render/Font.h"
#include "scene/UUID.h"
#include "physics/Physics.h"


#include <string>
#include <algorithm>

namespace trace {

	struct IDComponent
	{
		UUID _id;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct HierachyComponent
	{
		UUID parent = 0;
		std::vector<UUID> children;
		glm::mat4 transform = glm::identity<glm::mat4>();

		HierachyComponent() = default;
		HierachyComponent(const HierachyComponent&) = default;
		~HierachyComponent() {};

		void AddChild(UUID child)
		{
			auto it = std::find(children.begin(), children.end(), child);
			if (it == children.end())
			{
				children.emplace_back(child);
			}
		}

		void RemoveChild(UUID child)
		{
			auto it = std::find(children.begin(), children.end(), child);
			if (it != children.end())
			{
				children.erase(it);
			}
		}

		bool HasParent()
		{
			return (parent != 0);
		}

		bool HasChild(UUID child)
		{
			auto it = std::find(children.begin(), children.end(), child);
			return (it != children.end());
		}


	};

	struct TagComponent
	{
		std::string _tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) { _tag = name; }
		TagComponent(std::string& name) { _tag = name; }

	};

	struct TransformComponent
	{
		Transform _transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Transform& location)
			: _transform(location){}

		TransformComponent(Transform& location)
			: _transform(location) {}
	};


	struct CameraComponent
	{
		Camera _camera;
		bool is_main = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Camera& location)
			: _camera(location), is_main(false) {}

		CameraComponent(const Camera& location, bool main)
			: _camera(location), is_main(main) {}

		CameraComponent(Camera& location)
			: _camera(location), is_main(false) {}
	};

	struct LightComponent
	{
		Light _light;
		LightType light_type;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(const Light& location)
			: _light(location), light_type(LightType::UNKNOWN) {}

		LightComponent(const Light& location, LightType type)
			: _light(location), light_type(type) {}


		LightComponent(Light& location)
			: _light(location), light_type(LightType::UNKNOWN) {}
	};

	struct MeshComponent
	{
		Ref<Mesh> _mesh;
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Ref<Mesh> data)
			:_mesh(data){}
	};

	struct ModelComponent
	{
		Ref<Model> _model;
		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;
		ModelComponent(Ref<Model> data)
			:_model(data) {}
		~ModelComponent()
		{

		}
	};

	struct ModelRendererComponent
	{
		Ref<MaterialInstance> _material;
		ModelRendererComponent() = default;
		ModelRendererComponent(const ModelRendererComponent&) = default;
		ModelRendererComponent(Ref<MaterialInstance> data)
			:_material(data) {}
		~ModelRendererComponent()
		{

		}
	};

	struct TextComponent
	{
		Ref<Font> font;
		std::string text;
		glm::vec3 color;
		float intensity = 1.0f;
		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(std::string data)
			:text(data) {}
		~TextComponent()
		{

		}
	};

	

	struct RigidBodyComponent
	{
		RigidBody body;
		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
		~RigidBodyComponent()
		{

		}
	};

	struct BoxColliderComponent
	{
		PhyShape shape;
		bool is_trigger = false;
		void* _internal = nullptr;
		BoxColliderComponent() { shape.SetBox(glm::vec3(0.5f)); shape.offset = glm::vec3(0.0f); };
		BoxColliderComponent(const BoxColliderComponent&) = default;
		~BoxColliderComponent()
		{

		}
	};

	struct SphereColliderComponent
	{
		PhyShape shape;
		bool is_trigger = false;
		void* _internal = nullptr;
		SphereColliderComponent() { shape.SetSphere(0.5f); shape.offset = glm::vec3(0.0f); };
		SphereColliderComponent(const SphereColliderComponent&) = default;
		~SphereColliderComponent()
		{

		}
	};





}
