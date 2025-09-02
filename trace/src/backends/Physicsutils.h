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
	typedef bool (*__CreateShapeWithTransform)(void*&,void*&, PhyShape&, Transform&,bool);
	typedef bool (*__DestroyShape)(void*&);
	typedef bool (*__SetShapePtr)(void*&, void*);
	typedef bool (*__SetShapeMask)(void*&, uint32_t, uint32_t);
	typedef bool (*__UpdateShapeTransform)(void*&, trace::PhyShape& geometry, Transform&);
	typedef bool (*__AttachShape)(void*&, void*&);
	typedef bool (*__DetachShape)(void*&, void*&);

	//Rigid Body
	typedef bool (*__CreateRigidBody)(RigidBody&, Transform&);
	typedef bool (*__CreateRigidBody_Scene)(void*&, RigidBody&, Transform&);
	typedef bool (*__DestroyRigidBody)(RigidBody&);
	typedef bool (*__SetRigidBodyTransform)(RigidBody&, Transform&);
	typedef bool (*__GetRigidBodyTransform)(RigidBody&, Transform&);

	//Character Controller
	typedef bool (*__CreateCharacterController)(CharacterController&, void*&, Transform&);
	typedef bool (*__DestroyCharacterController)(CharacterController&, void*&);
	typedef bool (*__MoveCharacterController)(CharacterController&, glm::vec3, float);
	typedef bool (*__GetCharacterControllerPosition)(CharacterController&, glm::vec3&);
	typedef bool (*__SetCharacterControllerPosition)(CharacterController&, glm::vec3&);
	typedef bool (*__SetControllerDataPtr)(CharacterController&, void*);

	//Queries
	typedef bool (*__RayCast)(void*, glm::vec3, glm::vec3, float, RaycastHit&);
	typedef bool (*__RayCastBox)(glm::vec3, glm::mat4, glm::vec3, glm::vec3, float, RaycastHit&);


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
		static bool CreateShapeWithTransform(void*& scene, void*& shape, PhyShape geometry, Transform& transform,bool trigger = false);
		static bool DestroyShape(void*& shape);
		static bool SetShapePtr(void*& shape, void* ptr);
		static bool SetShapeMask(void*& shape, uint32_t mask0, uint32_t mask1);
		static bool UpdateShapeTransform(void*& shape, trace::PhyShape& geometry, Transform& transform);
		static bool AttachShape(void*& shape, void*& actor);
		static bool DetachShape(void*& shape, void*& actor);

		static bool CreateRigidBody(RigidBody& rigid_body, Transform& transform);
		static bool CreateRigidBody_Scene(void*& scene, RigidBody& rigid_body, Transform& transform);
		static bool DestroyRigidBody(RigidBody& rigid_body);
		static bool SetRigidBodyTransform(RigidBody& rigid_body, Transform& transform);
		static bool GetRigidBodyTransform(RigidBody& rigid_body, Transform&);

		static bool CreateCharacterController(CharacterController& controller, void*& scene, Transform& pose);
		static bool DestroyCharacterController(CharacterController& controller, void*& scene);
		static bool MoveCharacterController(CharacterController& controller, glm::vec3 displacement, float deltaTime);
		static bool SetControllerDataPtr(CharacterController& controller, void* ptr);
		static bool GetCharacterControllerPosition(CharacterController& controller, glm::vec3& out_position);
		static bool SetCharacterControllerPosition(CharacterController& controller, glm::vec3& position);

		static bool RayCast(void* scene, glm::vec3 origin, glm::vec3 direction, float max_distance, RaycastHit& result);
		static bool RayCastBox(glm::vec3 half_extents, glm::mat4 pose, glm::vec3 origin, glm::vec3 direction, float max_distance, RaycastHit& result);



	private:

		static __InitPhysics3D _initPhysics3D;
		static __ShutdownPhysics3D _shutdownPhysics3D;

		static __CreateScene3D _createScene3D;
		static __DestroyScene3D _destroyScene3D;
		static __AddActor _addActor;
		static __RemoveActor _removeActor;
		static __Stimulate _stimulate;

		static __CreateShape _createShape;
		static __CreateShapeWithTransform _createShapeWithTransform;
		static __DestroyShape _destroyShape;
		static __SetShapePtr _setShapePtr;
		static __SetShapeMask _setShapeMask;
		static __UpdateShapeTransform _updateShapeTransform;
		static __AttachShape _attachShape;
		static __DetachShape _detachShape;

		static __CreateRigidBody _createRigidBody;
		static __CreateRigidBody_Scene _createRigidBody_Scene;
		static __DestroyRigidBody _destroyRigidBody;
		static __SetRigidBodyTransform _setRigidBodyTransform;
		static __GetRigidBodyTransform _getRigidBodyTransform;

		static __CreateCharacterController _createCharacterController;
		static __DestroyCharacterController _destroyCharacterController;
		static __MoveCharacterController _moveCharacterController;
		static __SetControllerDataPtr _setControllerDataPtr;
		static __GetCharacterControllerPosition _getCharacterControllerPosition;
		static __SetCharacterControllerPosition _setCharacterControllerPosition;

		static __RayCast _rayCast;
		static __RayCastBox _rayCastBox;


	protected:
		friend PhysicsFuncLoader;
	};

}
