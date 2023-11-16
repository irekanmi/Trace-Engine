#pragma once

#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"
#include "render/Mesh.h"
#include "render/Font.h"
#include "scene/UUID.h"
#include "physics/Physics.h"


#include <string>

namespace trace {

	struct IDComponent
	{
		UUID _id;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
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
		Ref<Mesh> _mesh;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(const Light& location)
			: _light(location), light_type(LightType::UNKNOWN) {}

		LightComponent(const Light& location, LightType type)
			: _light(location), light_type(type) {}

		LightComponent(const Light& location, LightType type, Ref<Mesh> mesh)
			: _light(location), light_type(type), _mesh(mesh) {}

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
		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(std::string data)
			:text(data) {}
		~TextComponent()
		{

		}
	};

	struct BoxCoillderComponent
	{
		PhyShape shape;
		bool is_trigger = false;
		void* _internal = nullptr;
		BoxCoillderComponent() { shape.SetBox(glm::vec3(0.5f)); shape.offset = glm::vec3(0.0f); };
		BoxCoillderComponent(const BoxCoillderComponent&) = default;
		~BoxCoillderComponent()
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



}
