#pragma once

#include "glm/glm.hpp"
#include "scene/UUID.h"

#include <xhash>

namespace trace {
	
	class Entity;

	class PhyShape
	{

	public:

		void SetBox(glm::vec3 extents)
		{
			type = Type::Box;
			box.half_extents = extents;
		}

		void SetSphere(float radius)
		{
			type = Type::Sphere;
			sphere.radius = radius;
		}

		void SetCapsule(float radius, float half_height)
		{
			type = Type::Capsule;
			capsule.radius = radius;
			capsule.half_height = half_height;
		}



		enum Type
		{
			None,
			Box,
			Sphere,
			Capsule
		};

		glm::vec3 offset;
		Type type = Type::None;

		
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

		};

	private:
	protected:

	};

	class RigidBody
	{

	public:
		enum Type
		{
			Static,
			Kinematic,
			Dynamic
		};

		Type GetType() { return m_type; }
		void SetType(Type type) { m_type = type; }
		void*& GetInternal() { return m_internal; }
		

		float mass = 1.0f;
		float density = 10.0f;

	private:
		Type m_type = Type::Static;
		void* m_internal = nullptr;

	protected:

	};

	using TriggerPair = std::pair<Entity, Entity>;
	using CollisionPair = std::pair<uint64_t, uint64_t>;

	

	struct ContactPoint
	{
		glm::vec3 point;
		glm::vec3 normal;
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
			otherEntity = entity;
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