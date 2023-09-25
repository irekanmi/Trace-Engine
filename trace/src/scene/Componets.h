#pragma once

#include <string>
#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"
#include "render/Mesh.h"

namespace trace {

	struct TagComponent
	{
		std::string _tag;

		TagComponent() = default;
		TagComponent(const std::string& name) { _tag = name; }
		TagComponent(std::string& name) { _tag = name; }

	};

	struct TransformComponent
	{
		Transform _transform;

		TransformComponent() = default;
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
		MeshComponent(Ref<Mesh> data)
			:_mesh(data){}
	};

}
