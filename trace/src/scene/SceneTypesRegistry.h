#pragma once

#include "reflection/TypeRegistry.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "scene/Components.h"


namespace trace {

	BEGIN_REGISTER_CLASS(IDComponent)
		REGISTER_TYPE(IDComponent);
		REGISTER_MEMBER(IDComponent, _id);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(HierachyComponent)
		REGISTER_TYPE(HierachyComponent);
		REGISTER_MEMBER(HierachyComponent, parent);
		REGISTER_MEMBER(HierachyComponent, children);
		REGISTER_MEMBER(HierachyComponent, is_enabled);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(TagComponent)
		REGISTER_TYPE(TagComponent);
		REGISTER_MEMBER(TagComponent, m_tag);
		REGISTER_MEMBER(TagComponent, m_id);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(TransformComponent)
		REGISTER_TYPE(TransformComponent);
		REGISTER_MEMBER(TransformComponent, _transform);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(PrefabComponent)
		REGISTER_TYPE(PrefabComponent);
		REGISTER_MEMBER(PrefabComponent, handle);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(CameraComponent)
		REGISTER_TYPE(CameraComponent);
		REGISTER_MEMBER(CameraComponent, _camera);
		REGISTER_MEMBER(CameraComponent, is_main);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(LightComponent)
		REGISTER_TYPE(LightComponent);
		REGISTER_MEMBER(LightComponent, _light);
		REGISTER_MEMBER(LightComponent, light_type);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ModelComponent)
		REGISTER_TYPE(ModelComponent);
		REGISTER_MEMBER(ModelComponent, _model);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ModelRendererComponent)
		REGISTER_TYPE(ModelRendererComponent);
		REGISTER_MEMBER(ModelRendererComponent, _material);
		REGISTER_MEMBER(ModelRendererComponent, cast_shadow);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SkinnedModelRenderer)
		REGISTER_TYPE(SkinnedModelRenderer);
		REGISTER_MEMBER(SkinnedModelRenderer, _material);
		REGISTER_MEMBER(SkinnedModelRenderer, _model);
		REGISTER_MEMBER(SkinnedModelRenderer, cast_shadow);
		REGISTER_MEMBER(SkinnedModelRenderer, runtime_skeleton);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(TextComponent)
		REGISTER_TYPE(TextComponent);
		REGISTER_MEMBER(TextComponent, font);
		REGISTER_MEMBER(TextComponent, text);
		REGISTER_MEMBER(TextComponent, color);
		REGISTER_MEMBER(TextComponent, intensity);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(RigidBodyComponent)
		REGISTER_TYPE(RigidBodyComponent);
		REGISTER_MEMBER(RigidBodyComponent, body);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(BoxColliderComponent)
		REGISTER_TYPE(BoxColliderComponent);
		REGISTER_MEMBER(BoxColliderComponent, shape);
		REGISTER_MEMBER(BoxColliderComponent, is_trigger);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SphereColliderComponent)
		REGISTER_TYPE(SphereColliderComponent);
		REGISTER_MEMBER(SphereColliderComponent, shape);
		REGISTER_MEMBER(SphereColliderComponent, is_trigger);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(AnimationComponent)
		REGISTER_TYPE(AnimationComponent);
		REGISTER_MEMBER(AnimationComponent, play_on_start);
		REGISTER_MEMBER(AnimationComponent, loop);
		REGISTER_MEMBER(AnimationComponent, animation);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ImageComponent)
		REGISTER_TYPE(ImageComponent);
		REGISTER_MEMBER(ImageComponent, image);
		REGISTER_MEMBER(ImageComponent, color);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SunLight)
		REGISTER_TYPE(SunLight);
		REGISTER_MEMBER(SunLight, color);
		REGISTER_MEMBER(SunLight, intensity);
		REGISTER_MEMBER(SunLight, cast_shadows);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(PointLight)
		REGISTER_TYPE(PointLight);
		REGISTER_MEMBER(PointLight, color);
		REGISTER_MEMBER(PointLight, intensity);
		REGISTER_MEMBER(PointLight, cast_shadows);
		REGISTER_MEMBER(PointLight, radius);
		REGISTER_MEMBER(PointLight, linear);
		REGISTER_MEMBER(PointLight, constant);
		REGISTER_MEMBER(PointLight, quadratic);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SpotLight)
		REGISTER_TYPE(SpotLight);
		REGISTER_MEMBER(SpotLight, color);
		REGISTER_MEMBER(SpotLight, intensity);
		REGISTER_MEMBER(SpotLight, cast_shadows);
		REGISTER_MEMBER(SpotLight, innerCutOff);
		REGISTER_MEMBER(SpotLight, outerCutOff);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SequencePlayer)
		REGISTER_TYPE(SequencePlayer);
		REGISTER_MEMBER(SequencePlayer, sequence);
		REGISTER_MEMBER(SequencePlayer, play_on_start);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(AnimationGraphController)
		REGISTER_TYPE(AnimationGraphController);
		REGISTER_MEMBER(AnimationGraphController, graph);
		REGISTER_MEMBER(AnimationGraphController, play_on_start);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(CharacterControllerComponent)
		REGISTER_TYPE(CharacterControllerComponent);
		REGISTER_MEMBER(CharacterControllerComponent, character);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(MotionMatchingComponent)
		REGISTER_TYPE(MotionMatchingComponent);
		REGISTER_MEMBER(MotionMatchingComponent, update_frequency);
		REGISTER_MEMBER(MotionMatchingComponent, trajectory_weight);
		REGISTER_MEMBER(MotionMatchingComponent, pose_weight);
		REGISTER_MEMBER(MotionMatchingComponent, normalized_search);
		REGISTER_MEMBER(MotionMatchingComponent, motion_matching_info);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SpringMotionMatchingController)
		REGISTER_TYPE(SpringMotionMatchingController);
		REGISTER_MEMBER(SpringMotionMatchingController, position_halflife);
		REGISTER_MEMBER(SpringMotionMatchingController, rotation_halflife);
	END_REGISTER_CLASS;

}
