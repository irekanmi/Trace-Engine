#include "pch.h"


#include "Physicsutils.h"
#include "core/io/Logging.h"
#include "core/platform/Physx/Physx_backend.h"


#define PHYSICS_FUNC_IS_VALID(function)                           \
	if(!function)                                                \
	{                                                            \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                           \
		return false;                                           \
	}


namespace trace {

	__InitPhysics3D PhysicsFunc::_initPhysics3D = nullptr;
	__ShutdownPhysics3D PhysicsFunc::_shutdownPhysics3D = nullptr;

	__CreateScene3D PhysicsFunc::_createScene3D = nullptr;
	__DestroyScene3D PhysicsFunc::_destroyScene3D = nullptr;
	__AddActor PhysicsFunc::_addActor = nullptr;
	__RemoveActor PhysicsFunc::_removeActor = nullptr;
	__Stimulate PhysicsFunc::_stimulate = nullptr;

	__CreateShape PhysicsFunc::_createShape = nullptr;
	__CreateShapeWithTransform PhysicsFunc::_createShapeWithTransform = nullptr;
	__DestroyShape PhysicsFunc::_destroyShape = nullptr;
	__SetShapePtr PhysicsFunc::_setShapePtr = nullptr;
	__SetShapeMask PhysicsFunc::_setShapeMask = nullptr;
	__UpdateShapeTransform PhysicsFunc::_updateShapeTransform = nullptr;
	__AttachShape PhysicsFunc::_attachShape = nullptr;
	__DetachShape PhysicsFunc::_detachShape = nullptr;

	__CreateRigidBody PhysicsFunc::_createRigidBody = nullptr;
	__CreateRigidBody_Scene PhysicsFunc::_createRigidBody_Scene = nullptr;
	__DestroyRigidBody PhysicsFunc::_destroyRigidBody = nullptr;
	__SetRigidBodyTransform PhysicsFunc::_setRigidBodyTransform = nullptr;
	__GetRigidBodyTransform PhysicsFunc::_getRigidBodyTransform = nullptr;

	__CreateCharacterController PhysicsFunc::_createCharacterController = nullptr;
	__DestroyCharacterController PhysicsFunc::_destroyCharacterController = nullptr;
	__MoveCharacterController PhysicsFunc::_moveCharacterController = nullptr;
	__SetControllerDataPtr PhysicsFunc::_setControllerDataPtr = nullptr;
	__GetCharacterControllerPosition PhysicsFunc::_getCharacterControllerPosition = nullptr;
	__SetCharacterControllerPosition PhysicsFunc::_setCharacterControllerPosition = nullptr;

	__RayCast PhysicsFunc::_rayCast = nullptr;

	// Loader ---------------------------------------
	bool PhysicsFuncLoader::LoadPhysxFunctions()
	{
		PhysicsFunc::_initPhysics3D = physx::__InitPhysics3D;
		PhysicsFunc::_shutdownPhysics3D = physx::__ShutdownPhysics3D;

		PhysicsFunc::_createScene3D = physx::__CreateScene3D;
		PhysicsFunc::_destroyScene3D = physx::__DestroyScene3D;
		PhysicsFunc::_addActor = physx::__AddActor;
		PhysicsFunc::_removeActor = physx::__RemoveActor;
		PhysicsFunc::_stimulate = physx::__Stimulate;

		PhysicsFunc::_createShape = physx::__CreateShape;
		PhysicsFunc::_destroyShape = physx::__DestroyShape;
		PhysicsFunc::_createShapeWithTransform = physx::__CreateShapeWithTransform;
		PhysicsFunc::_updateShapeTransform = physx::__UpdateShapeTransform;
		PhysicsFunc::_setShapePtr = physx::__SetShapePtr;
		PhysicsFunc::_setShapeMask = physx::__SetShapeMask;
		PhysicsFunc::_attachShape = physx::__AttachShape;
		PhysicsFunc::_detachShape = physx::__DetachShape;

		PhysicsFunc::_createRigidBody = physx::__CreateRigidBody;
		PhysicsFunc::_createRigidBody_Scene = physx::__CreateRigidBody_Scene;
		PhysicsFunc::_destroyRigidBody = physx::__DestroyRigidBody;
		PhysicsFunc::_setRigidBodyTransform = physx::__SetRigidBodyTransform;
		PhysicsFunc::_getRigidBodyTransform = physx::__GetRigidBodyTransform;

		PhysicsFunc::_createCharacterController = physx::__CreateCharacterController;
		PhysicsFunc::_destroyCharacterController = physx::__DestroyCharacterController;
		PhysicsFunc::_moveCharacterController = physx::__MoveCharacterController;
		PhysicsFunc::_setControllerDataPtr = physx::__SetControllerDataPtr;
		PhysicsFunc::_getCharacterControllerPosition = physx::__GetCharacterControllerPosition;
		PhysicsFunc::_setCharacterControllerPosition = physx::__SetCharacterControllerPosition;

		PhysicsFunc::_rayCast = physx::__RayCast;

		return true;
	}

	bool PhysicsFuncLoader::LoadBox2dFunctions()
	{
		return true;
	}
	// ----------------------------------------------

	//
	bool PhysicsFunc::InitPhysics3D()
	{
		PHYSICS_FUNC_IS_VALID(_initPhysics3D);
		return _initPhysics3D();
	}

	bool PhysicsFunc::ShutdownPhysics3D()
	{
		PHYSICS_FUNC_IS_VALID(_shutdownPhysics3D);
		return _shutdownPhysics3D();
	}

	bool PhysicsFunc::CreateScene3D(void*& scene, glm::vec3 gravity)
	{
		PHYSICS_FUNC_IS_VALID(_createScene3D);
		return _createScene3D(scene, gravity);
	}

	bool PhysicsFunc::DestroyScene3D(void*& scene)
	{
		PHYSICS_FUNC_IS_VALID(_destroyScene3D);
		return _destroyScene3D(scene);
	}

	bool PhysicsFunc::AddActor(void*& scene, void*& actor)
	{
		PHYSICS_FUNC_IS_VALID(_addActor);
		return _addActor(scene, actor);
	}

	bool PhysicsFunc::RemoveActor(void*& scene, void*& actor)
	{
		PHYSICS_FUNC_IS_VALID(_removeActor);
		return _removeActor(scene, actor);
	}

	bool PhysicsFunc::Stimulate(void*& scene, float deltaTime)
	{
		PHYSICS_FUNC_IS_VALID(_stimulate);
		return _stimulate(scene, deltaTime);
	}

	bool PhysicsFunc::CreateShape(void*& shape, PhyShape geometry, bool trigger)
	{
		PHYSICS_FUNC_IS_VALID(_createShape);
		return _createShape(shape, geometry, trigger);
	}

	bool PhysicsFunc::CreateShapeWithTransform(void*& scene, void*& shape, PhyShape geometry, Transform& transform, bool trigger)
	{
		PHYSICS_FUNC_IS_VALID(_createShapeWithTransform);
		return _createShapeWithTransform(scene, shape, geometry, transform, trigger);
	}

	bool PhysicsFunc::DestroyShape(void*& shape)
	{
		PHYSICS_FUNC_IS_VALID(_destroyShape);
		return _destroyShape(shape);
	}

	bool PhysicsFunc::SetShapePtr(void*& shape, void* ptr)
	{
		PHYSICS_FUNC_IS_VALID(_setShapePtr);
		return _setShapePtr(shape, ptr);
	}

	bool PhysicsFunc::SetShapeMask(void*& shape, uint32_t mask0, uint32_t mask1)
	{
		PHYSICS_FUNC_IS_VALID(_setShapeMask);
		return _setShapeMask(shape, mask0, mask1);
	}

	bool PhysicsFunc::UpdateShapeTransform(void*& shape, Transform& transform)
	{
		PHYSICS_FUNC_IS_VALID(_updateShapeTransform);
		return _updateShapeTransform(shape, transform);
	}

	bool PhysicsFunc::AttachShape(void*& shape, void*& actor)
	{
		PHYSICS_FUNC_IS_VALID(_attachShape);
		return _attachShape(shape, actor);
	}

	bool PhysicsFunc::DetachShape(void*& shape, void*& actor)
	{
		PHYSICS_FUNC_IS_VALID(_detachShape);
		return _detachShape(shape, actor);
	}

	bool PhysicsFunc::CreateRigidBody(RigidBody& rigid_body, Transform& transform)
	{
		PHYSICS_FUNC_IS_VALID(_createRigidBody);
		return _createRigidBody(rigid_body,transform);
	}

	bool PhysicsFunc::CreateRigidBody_Scene(void*& scene, RigidBody& rigid_body, Transform& transform)
	{
		PHYSICS_FUNC_IS_VALID(_createRigidBody_Scene);
		return _createRigidBody_Scene(scene, rigid_body, transform);
	}

	bool PhysicsFunc::DestroyRigidBody(RigidBody& rigid_body)
	{
		PHYSICS_FUNC_IS_VALID(_destroyRigidBody);
		return _destroyRigidBody(rigid_body);
	}

	bool PhysicsFunc::SetRigidBodyTransform(RigidBody& rigid_body, Transform& transform)
	{
		PHYSICS_FUNC_IS_VALID(_setRigidBodyTransform);
		return _setRigidBodyTransform(rigid_body, transform);
	}

	bool PhysicsFunc::GetRigidBodyTransform(RigidBody& rigid_body, Transform& transform)
	{
		PHYSICS_FUNC_IS_VALID(_getRigidBodyTransform);
		return _getRigidBodyTransform(rigid_body, transform);
	}

	bool PhysicsFunc::CreateCharacterController(CharacterController& controller, void*& scene, Transform& pose)
	{
		PHYSICS_FUNC_IS_VALID(_createCharacterController);
		return _createCharacterController(controller, scene, pose);
	}

	bool PhysicsFunc::DestroyCharacterController(CharacterController& controller, void*& scene)
	{
		PHYSICS_FUNC_IS_VALID(_destroyCharacterController);
		return _destroyCharacterController(controller, scene);
	}

	bool PhysicsFunc::MoveCharacterController(CharacterController& controller, glm::vec3 displacement, float deltaTime)
	{
		PHYSICS_FUNC_IS_VALID(_moveCharacterController);
		return _moveCharacterController(controller, displacement, deltaTime);
	}

	bool PhysicsFunc::SetControllerDataPtr(CharacterController& controller, void* ptr)
	{
		PHYSICS_FUNC_IS_VALID(_setControllerDataPtr);
		return _setControllerDataPtr(controller, ptr);
	}

	bool PhysicsFunc::GetCharacterControllerPosition(CharacterController& controller, glm::vec3& out_position)
	{
		PHYSICS_FUNC_IS_VALID(_getCharacterControllerPosition);
		return _getCharacterControllerPosition(controller, out_position);
	}
	
	bool PhysicsFunc::SetCharacterControllerPosition(CharacterController& controller, glm::vec3& position)
	{
		PHYSICS_FUNC_IS_VALID(_setCharacterControllerPosition);
		return _setCharacterControllerPosition(controller, position);
	}

	bool PhysicsFunc::RayCast(void* scene, glm::vec3 origin, glm::vec3 direction, float max_distance, RaycastHit& result)
	{
		PHYSICS_FUNC_IS_VALID(_rayCast);
		return _rayCast(scene, origin, direction, max_distance, result);
	}

}






