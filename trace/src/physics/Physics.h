#pragma once

#include "glm/glm.hpp"

namespace trace {

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

}