#pragma once

#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"
#include "render/Mesh.h"
#include "render/Font.h"
#include "scene/UUID.h"
#include "physics/Physics.h"
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "resource/Prefab.h"
#include "core/Enums.h"
#include "render/SkinnedModel.h"
#include "animation/Skeleton.h"
#include "core/Coretypes.h"
#include "animation/AnimationSequence.h"


#include <string>
#include <unordered_map>
#include <algorithm>

namespace trace {

	class Scene;

	struct ActiveComponent
	{
		bool is_active = true;
	};

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
		bool is_enabled = true;

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

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) { m_tag = name; }
		TagComponent(std::string& name) { m_tag = name; }

		std::string& GetTag() { return m_tag; }
		StringID GetStringID() { return m_id; }
		void SetTag(const std::string& name);

	private:
		std::string m_tag;
		StringID m_id;

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

	struct PrefabComponent
	{
		Ref<Prefab> handle;

		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent&) = default;
		PrefabComponent(const Ref<Prefab> data)
			: handle(data) {}
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
		bool cast_shadow = true;
		ModelRendererComponent() = default;
		ModelRendererComponent(const ModelRendererComponent&) = default;
		ModelRendererComponent(Ref<MaterialInstance> data)
			:_material(data) {}
		~ModelRendererComponent()
		{

		}
	};

	struct SkinnedModelRenderer
	{
		Ref<MaterialInstance> _material;
		Ref<SkinnedModel> _model;
		bool cast_shadow = true;
		std::vector<glm::mat4> bone_transforms;
		Animation::SkeletonInstance runtime_skeleton;

		Ref<Animation::Skeleton> GetSkeleton() { return runtime_skeleton.GetSkeleton(); }
		void SetSkeleton(Ref<Animation::Skeleton> skeleton, Scene* scene, UUID parent);

	private:

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

	struct AnimationComponent
	{
		bool play_on_start = false;
		bool loop = false;
		std::unordered_map<StringID, UUID> entities;
		Ref<AnimationClip> animation;
		float elasped_time = 0.0f;
		bool started = false;

		bool InitializeEntities(Scene* scene);
		void Start();
		void Stop();
		void Pause();
		void Update(float deltaTime, Scene* scene);
	};

	struct ImageComponent
	{
		Ref<GTexture> image;
		uint32_t color = TRC_COL32_WHITE;
	};

	struct SunLight
	{
		glm::vec3 color;
		float intensity = 1.0f;
		bool cast_shadows = false;
	};

	struct PointLight
	{
		glm::vec3 color;
		float intensity = 1.0f;
		float radius = 2.0f;
		float constant = 1.0f;
		float linear = 0.02f;
		float quadratic = 0.0001f;
		bool cast_shadows = false;
	};

	struct SpotLight
	{
		glm::vec3 color;
		float intensity = 1.0f;
		float innerCutOff = 0.8660f;
		float outerCutOff = 0.7071f;
		bool cast_shadows = false;
	};

	struct SequencePlayer
	{
		Animation::SequenceInstance sequence;
		bool play_on_start = false;
	};

	struct AnimationGraphController
	{
		Animation::GraphInstance graph;
		bool play_on_start = false;
	};

	template<typename... Component>
	struct ComponentGroup
	{

	};

	using AllComponents = ComponentGroup<TagComponent, TransformComponent, CameraComponent,
		LightComponent, MeshComponent, ModelComponent, ModelRendererComponent, TextComponent, RigidBodyComponent,
		BoxColliderComponent, SphereColliderComponent, AnimationComponent, ImageComponent, PrefabComponent, SunLight,
		PointLight, SpotLight, SkinnedModelRenderer, SequencePlayer, AnimationGraphController>;

}
