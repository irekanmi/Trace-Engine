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
	bool __CreateShapeWithTransform(void*& scene,void*& shape, trace::PhyShape& geometry, trace::Transform& transform, bool trigger = false);
	bool __UpdateShapeTransform(void*& shape, trace::Transform& transform);
	bool __DestroyShape(void*& shape);
	bool __SetShapePtr(void*& shape, void* ptr);
	bool __SetShapeMask(void*& shape, uint32_t mask0, uint32_t mask1);
	bool __AttachShape(void*& shape, void*& actor);
	bool __DetachShape(void*& shape, void*& actor);
	bool __CreateRigidBody(trace::RigidBody& rigid_body, trace::Transform& transform);
	bool __CreateRigidBody_Scene(void*& scene,trace::RigidBody& rigid_body, trace::Transform& transform);
	bool __DestroyRigidBody(trace::RigidBody& rigid_body);
	bool __SetRigidBodyTransform(trace::RigidBody& rigid_body, trace::Transform& transform);
	bool __GetRigidBodyTransform(trace::RigidBody& rigid_body, trace::Transform& transform);
	bool __CreateCharacterController(trace::CharacterController& controller, void*& scene, trace::Transform& pose);
	bool __DestroyCharacterController(trace::CharacterController& controller, void*& scene);
	bool __MoveCharacterController(trace::CharacterController& controller, glm::vec3 displacement, float deltaTime);
	bool __SetControllerDataPtr(trace::CharacterController& controller, void* ptr);
	bool __GetCharacterControllerPosition(trace::CharacterController& controller, glm::vec3& out_position);

}
