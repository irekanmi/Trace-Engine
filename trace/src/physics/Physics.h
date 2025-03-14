#pragma once

#include "scene/UUID.h"
#include "reflection/TypeRegistry.h"


#include "glm/glm.hpp"

#include <xhash>

namespace trace {
	enum class PhyShapeType
	{
		None,
		Box,
		Sphere,
		Capsule
	};

	class PhyShape
	{

	public:

		void SetBox(glm::vec3 extents)
		{
			type = PhyShapeType::Box;
			box.half_extents = extents;
		}

		void SetSphere(float radius)
		{
			type = PhyShapeType::Sphere;
			sphere.radius = radius;
		}

		void SetCapsule(float radius, float half_height)
		{
			type = PhyShapeType::Capsule;
			capsule.radius = radius;
			capsule.half_height = half_height;
		}





		glm::vec3 offset;
		PhyShapeType type = PhyShapeType::None;

		
	public:
		

		union 
		{

			struct Box
			{
				glm::vec3 half_extents;
			} box;

			struct Sphere
			{
				float radius;
			} sphere;

			struct Capsule
			{
				float radius;
				float half_height;
			} capsule;

			char data[16];//NOTE: Used by the reflection system to serialize data
		};

	private:
	protected:

	};

	enum class RigidBodyType
	{
		Static,
		Kinematic,
		Dynamic
	};

	class RigidBody
	{

	public:
		

		RigidBodyType GetType() { return m_type; }
		void SetType(RigidBodyType type) { m_type = type; }
		void*& GetInternal() { return m_internal; }


		float mass = 1.0f;
		float density = 10.0f;

	private:
		RigidBodyType m_type = RigidBodyType::Static;
		void* m_internal = nullptr;

	protected:
		ACCESS_CLASS_MEMBERS(RigidBody);

	};

	class CharacterController
	{
	public:

		void SetInternal(void* handle) { m_internal = handle; }
		void* GetInternal() { return m_internal; }

		void SetIsGrounded(bool grounded) { m_isGrounded = grounded; }
		bool GetIsGrounded() { return m_isGrounded; }

		float height = 1.0f;
		float radius = 1.0f;
		float min_move_distance = 0.001f;
		float slope_limit = 45.0f;
		float contact_offset = 0.01f;
		float step_offset = 0.1f;
		glm::vec3 offset = glm::vec3(0.0f);
	private:
		bool m_isGrounded = false;
		void* m_internal = nullptr;

	protected:
		ACCESS_CLASS_MEMBERS(CharacterController);

	};

	using CollisionPair = std::pair<uint64_t, uint64_t>;

	struct TriggerPair
	{
		UUID entity = 0;
		UUID otherEntity = 0;

		void Swap()
		{
			UUID temp = entity;
			entity = otherEntity;
			otherEntity = temp;
		}
	};

	struct ContactPoint
	{
		glm::vec3 point = glm::vec3(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);
		float seperation;
	};

	// The maximum amount of the contact points to be stored within a single collision data (higher amount will be skipped).
#define COLLISION_MAX_CONTACT_POINTS 8

	struct CollisionData
	{
		UUID entity;
		UUID otherEntity;
		glm::vec3 impulse;
		uint32_t numContacts;
		ContactPoint contacts[COLLISION_MAX_CONTACT_POINTS];

		void Swap()
		{
			UUID temp = entity;
			entity = otherEntity;
			otherEntity = temp;
		}
	};

}

namespace std {

	template<>
	struct hash<trace::CollisionPair>
	{

		std::size_t operator()(const trace::CollisionPair& pair) const
		{
			return hash<uint64_t>()(pair.first) ^ hash<uint64_t>()(pair.second);
		}

	};
}