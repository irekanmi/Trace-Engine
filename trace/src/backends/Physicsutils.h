#pragma once

#include "physics/Physics.h"
#include "render/Transform.h"

#include "glm/glm.hpp"


namespace trace {

	typedef bool (*__InitPhysics3D)();
	typedef bool (*__ShutdownPhysics3D)();

	//Scene
	typedef bool (*__CreateScene3D)(void*&, glm::vec3);
	typedef bool (*__DestroyScene3D)(void*&);
	typedef bool (*__AddActor)(void*&, void*&);
	typedef bool (*__RemoveActor)(void*&, void*&);
	typedef bool (*__Stimulate)(void*&,float);

	//Shapes
	typedef bool (*__CreateShape)(void*&, PhyShape&, bool);
	typedef bool (*__AttachShape)(void*&, void*&);
	typedef bool (*__DetachShape)(void*&, void*&);

	//Rigid Body
	typedef bool (*__CreateRigidBody)(RigidBody&, Transform&);
	typedef bool (*__DestroyRigidBody)(RigidBody&);
	typedef bool (*__GetRigidBodyTransform)(RigidBody&, Transform&);

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

		static bool CreateScene3D(void*& scene, glm::vec3 gravity);
		static bool DestroyScene3D(void*& scene);
		static bool AddActor(void*& scene,void*& actor);
		static bool RemoveActor(void*& scene, void*& actor);
		static bool Stimulate(void*& scene, float deltaTime);

		static bool CreateShape(void*& shape, PhyShape geometry, bool trigger = false);
		static bool AttachShape(void*& shape, void*& actor);
		static bool DetachShape(void*& shape, void*& actor);

		static bool CreateRigidBody(RigidBody& rigid_body, Transform& transform);
		static bool DestroyRigidBody(RigidBody& rigid_body);
		static bool GetRigidBodyTransform(RigidBody& rigid_body, Transform&);



	private:

		static __InitPhysics3D _initPhysics3D;
		static __ShutdownPhysics3D _shutdownPhysics3D;

		static __CreateScene3D _createScene3D;
		static __DestroyScene3D _destroyScene3D;
		static __AddActor _addActor;
		static __RemoveActor _removeActor;
		static __Stimulate _stimulate;

		static __CreateShape _createShape;
		static __AttachShape _attachShape;
		static __DetachShape _detachShape;

		static __CreateRigidBody _createRigidBody;
		static __DestroyRigidBody _destroyRigidBody;
		static __GetRigidBodyTransform _getRigidBodyTransform;


	protected:
		friend PhysicsFuncLoader;
	};

}
