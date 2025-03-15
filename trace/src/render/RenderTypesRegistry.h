#pragma once

#include "reflection/TypeRegistry.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"

namespace trace {

	BEGIN_REGISTER_CLASS(Transform)
		REGISTER_TYPE(Transform);
		REGISTER_MEMBER(Transform, m_rotation);
		REGISTER_MEMBER(Transform, m_position);
		REGISTER_MEMBER(Transform, m_scale);
	END_REGISTER_CLASS;

	REGISTER_TYPE(CameraType);

	BEGIN_REGISTER_CLASS(Camera)
		REGISTER_TYPE(Camera);
		REGISTER_MEMBER(Camera, m_position);
		REGISTER_MEMBER(Camera, m_lookDirection);
		REGISTER_MEMBER(Camera, m_upDirection);
		REGISTER_MEMBER(Camera, m_fov);
		REGISTER_MEMBER(Camera, m_zNear);
		REGISTER_MEMBER(Camera, m_zFar);
		REGISTER_MEMBER(Camera, m_orthographicSize);
		REGISTER_MEMBER(Camera, m_screenWidth);
		REGISTER_MEMBER(Camera, m_screenHeight);
		REGISTER_MEMBER(Camera, m_type);
	END_REGISTER_CLASS;

	REGISTER_TYPE(LightType);

	BEGIN_REGISTER_CLASS(Light)
		REGISTER_TYPE(Light);
		REGISTER_MEMBER(Light, position);
		REGISTER_MEMBER(Light, direction );
		REGISTER_MEMBER(Light, color );
		REGISTER_MEMBER(Light, params1 );
		REGISTER_MEMBER(Light, params2);
	END_REGISTER_CLASS;


	REGISTER_TYPE(SkinnedVertex);

	BEGIN_REGISTER_CLASS(SkinnedModel)
		REGISTER_TYPE(SkinnedModel);
		REGISTER_MEMBER(SkinnedModel, m_vertices);
		REGISTER_MEMBER(SkinnedModel, m_indices);
	END_REGISTER_CLASS;

	REGISTER_TYPE(Vertex);

	BEGIN_REGISTER_CLASS(Model)
		REGISTER_TYPE(Model);
		REGISTER_MEMBER(Model, m_vertices);
		REGISTER_MEMBER(Model, m_indices);
	END_REGISTER_CLASS;
		

}
