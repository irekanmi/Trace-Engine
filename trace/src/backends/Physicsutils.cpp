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

	// Loader ---------------------------------------
	bool PhysicsFuncLoader::LoadPhysxFunctions()
	{
		PhysicsFunc::_initPhysics3D = physx::__InitPhysics3D;
		PhysicsFunc::_shutdownPhysics3D = physx::__ShutdownPhysics3D;
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

	bool PhysicsFunc::CreateScene3D(void* scene, glm::vec3 gravity)
	{
		PHYSICS_FUNC_IS_VALID(_createScene3D)
		return _createScene3D(scene, gravity);
	}

	bool PhysicsFunc::DestroyScene3D(void* scene)
	{
		PHYSICS_FUNC_IS_VALID(_destroyScene3D);
		return _destroyScene3D(scene);
	}

}






