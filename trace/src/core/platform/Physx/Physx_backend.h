#pragma once

#include "physics/Physics.h"
#include "render/Transform.h"

#include "glm/glm.hpp"

namespace physx {


	bool __InitPhysics3D();
	bool __ShutdownPhysics3D();
	bool __CreateScene3D(void*& scene, glm::vec3 gravity);
	bool __DestroyScene3D(void*& scene);
	bool __AddActor(void*& scene, void*& actor);
	bool __RemoveActor(void*& scene, void*& actor);
	bool __Stimulate(void*& scene, float deltaTime);
	bool __CreateShape(void*& shape, trace::PhyShape& geometry, bool trigger = false);
	bool __AttachShape(void*& shape, void*& actor);
	bool __DetachShape(void*& shape, void*& actor);
	bool __CreateRigidBody(trace::RigidBody& rigid_body, trace::Transform& transform);
	bool __DestroyRigidBody(trace::RigidBody& rigid_body);
	bool __GetRigidBodyTransform(trace::RigidBody& rigid_body, trace::Transform& transform);

}
