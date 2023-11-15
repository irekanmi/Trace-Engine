#pragma once

#include "glm/glm.hpp"

namespace trace {

	typedef bool (*__InitPhysics3D)();
	typedef bool (*__ShutdownPhysics3D)();

	//Scene
	typedef bool (*__CreateScene3D)(void*, glm::vec3);
	typedef bool (*__DestroyScene3D)(void*);

	class PhysicsFuncLoader
	{
	public:
		static bool LoadPhysxFunctions();
		static bool LoadBox2dFunctions();
	};

	class PhysicsFunc
	{

	public:
		static bool InitPhysics3D();
		static bool ShutdownPhysics3D();

		static bool CreateScene3D(void* scene, glm::vec3 gravity);
		static bool DestroyScene3D(void* scene);


	private:

		static __InitPhysics3D _initPhysics3D;
		static __ShutdownPhysics3D _shutdownPhysics3D;
		static __CreateScene3D _createScene3D;
		static __DestroyScene3D _destroyScene3D;


	protected:
		friend PhysicsFuncLoader;
	};

}
