#include "pch.h"

#include "Physx_backend.h"
#include "core/io/Logging.h"
#include "glm/gtc/quaternion.hpp"
#include "scene/Entity.h"
#include "scene/Componets.h"


// HACK: Fix "Physx\foundation\PxPreprocessor.h(443,1): fatal error C1189: #error:  Exactly one of NDEBUG and _DEBUG needs to be defined!"
#ifdef TRC_DEBUG_BUILD
#define NDEBUG
#else
#define NDEBUG
#endif
//
#include "PxPhysicsAPI.h"


namespace physx {


	class SceneEvnetCallback : public PxSimulationEventCallback
	{

	public:

		virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override
		{

		}

		virtual void onWake(PxActor** actors, PxU32 count) override
		{

		}

		virtual void onSleep(PxActor** actors, PxU32 count) override
		{

		}

		virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
		{

		}

		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override
		{
			for (PxU32 i = 0; i < count; i++)
			{
				const PxTriggerPair& pair = pairs[i];

				// Ignore pairs when shapes have been deleted
				if (pair.flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
					continue;

				auto trigger = static_cast<trace::Entity*>(pair.triggerShape->userData);
				auto otherCollider = static_cast<trace::Entity*>(pair.otherShape->userData);
				TRC_ASSERT(trigger && otherCollider, "Invalid trigger pair");
				trace::CollisionPair colliders_pair = std::make_pair(*trigger, *otherCollider);

				if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					ExitTriggers.emplace_back(colliders_pair);
				}
				else
				{
					EnterTriggers.emplace_back(colliders_pair);
				}
			}
		}

		virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
		{

		}

		void SendEvents()
		{
			// Temp __________________

			for (auto& i : EnterTriggers)
			{
				std::string first = i.first.GetComponent<trace::TagComponent>()._tag;
				std::string second = i.second.GetComponent<trace::TagComponent>()._tag;
				TRC_INFO(" '{}' entered these -> {}", first, second);
			}

			for (auto& i : ExitTriggers)
			{
				std::string first = i.first.GetComponent<trace::TagComponent>()._tag;
				std::string second = i.second.GetComponent<trace::TagComponent>()._tag;
				TRC_INFO(" '{}' exited these -> {}", first, second);
			}

			// _______________________
		}

		void Clear()
		{
			EnterTriggers.clear();
			ExitTriggers.clear();
		}

	private:
		std::vector<trace::CollisionPair> EnterTriggers;
		std::vector<trace::CollisionPair> ExitTriggers;

	protected:

	};

	class Physx_ErrorCallback : public PxErrorCallback
	{
	public:
		virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override
		{
			switch (code)
			{
			case PxErrorCode::eABORT:
			{
				TRC_CRITICAL("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			case PxErrorCode::eDEBUG_INFO:
			{
				TRC_INFO("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			case PxErrorCode::eDEBUG_WARNING:
			{
				TRC_WARN("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			case PxErrorCode::eINTERNAL_ERROR:
			{
				TRC_ERROR("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			case PxErrorCode::eINVALID_OPERATION:
			{
				TRC_TRACE("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			case PxErrorCode::eINVALID_PARAMETER:
			{
				TRC_TRACE("{}, -> File : {}, line : {}", message, file, line);
				break;
			}
			}
		}
	private:
	protected:
	};

	struct PhysxInternal
	{
		PxPhysics* physics = nullptr;
		PxFoundation* foundation = nullptr;
		PxDefaultCpuDispatcher* cpu_dispatcher = nullptr;
		PxPvd* pvd = nullptr;
		Physx_ErrorCallback error_cb;
		PxDefaultAllocator allocator;
		PxMaterial* default_material;
	};

	struct PhysxScene
	{
		PxScene* scene = nullptr;
		SceneEvnetCallback event_callback;
	};

	struct PhysxShape
	{
		PxShape* shp = nullptr;
		PxRigidActor* actor = nullptr;
		PxScene* scene = nullptr;
	};

	PxFilterFlags FilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		const bool maskTest = (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1);

		// Let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			if (maskTest)
			{
				// Notify trigger if masks specify it
				pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
				pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			}
			pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT; //TODO: Check Docks to understand Flag
			return PxFilterFlag::eDEFAULT;
		}

		//// Send events for the kinematic actors but don't solve the contact
		//if (PxFilterObjectIsKinematic(attributes0) && PxFilterObjectIsKinematic(attributes1))
		//{
		//	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		//	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		//	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
		//	pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
		//	return PxFilterFlag::eSUPPRESS;
		//}

		// Trigger the contact callback for pairs (A,B) where the filtermask of A contains the ID of B and vice versa
		if (maskTest)
		{
			pairFlags |= PxPairFlag::eSOLVE_CONTACT;
			pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
			pairFlags |= PxPairFlag::ePOST_SOLVER_VELOCITY;
			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return PxFilterFlag::eDEFAULT;
		}

		// Ignore pair (no collisions nor events)
		return PxFilterFlag::eKILL;
	}

	


	static PxVec3 Glm3ToPx3(glm::vec3& val)
	{
		return PxVec3( val.x, val.y, val.z );
	}

	static glm::vec3 Px3ToGlm3(PxVec3& val)
	{
		return glm::vec3(val.x, val.y, val.z);
	}

	static PxQuat GlmQuatToPxQuat(glm::quat& val)
	{
		return PxQuat(val.x, val.y, val.z, val.w);
	}

	static glm::quat PxQuatToGlmQuat(PxQuat& val)
	{
		return glm::quat(val.w, val.x, val.y, val.z);
	}

	static PhysxInternal phy;
	uint32_t num_threads = 2; //TODO: Configurable


	bool __InitPhysics3D()
	{
		phy.foundation = PxCreateFoundation(PX_PHYSICS_VERSION, phy.allocator, phy.error_cb);

		phy.pvd = PxCreatePvd(*phy.foundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		phy.pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

		phy.physics = PxCreatePhysics(PX_PHYSICS_VERSION, *phy.foundation, PxTolerancesScale(), true, phy.pvd);

		phy.default_material = phy.physics->createMaterial(0.5f, 0.5f, 0.6f);
		phy.cpu_dispatcher = PxDefaultCpuDispatcherCreate(num_threads);


		return true;
	}

	bool __ShutdownPhysics3D()
	{

		PX_RELEASE(phy.default_material);
		PX_RELEASE(phy.cpu_dispatcher);
		PX_RELEASE(phy.physics);
		if (phy.pvd)
		{
			PxPvdTransport* transport = phy.pvd->getTransport();
			phy.pvd->release();	phy.pvd = nullptr;
			PX_RELEASE(transport);
		}
		PX_RELEASE(phy.foundation);

		return true;
	}
	bool __CreateScene3D(void*& scene, glm::vec3 gravity)
	{
		PxSceneDesc desc(phy.physics->getTolerancesScale());
		PhysxScene* out_scene = new PhysxScene;//TODO: Use custom allocator
		desc.cpuDispatcher = phy.cpu_dispatcher;
		desc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		desc.filterShader = FilterShader;
		desc.simulationEventCallback = &out_scene->event_callback;

		PxScene* res = phy.physics->createScene(desc);
		if (res)
		{		
			out_scene->scene = res;
			scene = out_scene;
		}
		else
		{
			delete out_scene;
			scene = nullptr;
			return false;
		}


		return true;
	}
	bool __DestroyScene3D(void*& scene)
	{
		if (!scene)
		{
			TRC_ERROR("Enter a vaild physics scene ptr, file : {}, line : {}", __FILE__, __LINE__);
			return false;
		}

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);


		PxScene* res = in_scene->scene;
		PX_RELEASE(res);
		delete in_scene;
		scene = nullptr;

		return true;
	}
	bool __AddActor(void*& scene, void*& actor)
	{
		if (!scene || !actor)
		{
			TRC_WARN("Enter a valid scene or actor");
			return false;
		}

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);

		PxScene* scn = in_scene->scene;
		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);

		scn->addActor(*body);

		return true;
	}
	bool __RemoveActor(void*& scene, void*& actor)
	{
		if (!scene || !actor)
		{
			TRC_WARN("Enter a valid scene or actor");
			return false;
		}

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);
		
		PxScene* scn = in_scene->scene;
		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);

		scn->removeActor(*body);

		return true;
	}
	bool __Stimulate(void*& scene, float deltaTime)
	{
		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);

		PxScene* scn = in_scene->scene;
		scn->simulate(deltaTime);
		scn->fetchResults(true);

		in_scene->event_callback.SendEvents();
		in_scene->event_callback.Clear();

		return true;
	}
	bool __CreateShape(void*& shape, trace::PhyShape& geometry, bool trigger)
	{
		if (shape)
		{
			TRC_WARN("Shape has already been created , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PxShape* res = nullptr;
		switch (geometry.type)
		{
		case trace::PhyShape::Box:
		{
			PxVec3 extents = { geometry.box.half_extents.x, geometry.box.half_extents.y, geometry.box.half_extents.z };
			res = phy.physics->createShape(PxBoxGeometry(extents), *phy.default_material);
			break;
		}
		case trace::PhyShape::Sphere:
		{
			float radius = geometry.sphere.radius;
			res = phy.physics->createShape(PxSphereGeometry(radius), *phy.default_material);
			break;
		}
		case trace::PhyShape::Capsule:
		{
			float radius = geometry.capsule.radius;
			float half_height = geometry.capsule.half_height;
			res = phy.physics->createShape(PxCapsuleGeometry(radius, half_height), *phy.default_material);
			break;
		}
		}

		if (!res)
		{
			TRC_ERROR("Unable to create Shape");
			return false;
		}
		
		if (trigger)
		{
			res->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			res->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		 PhysxShape* result = new PhysxShape(); //TODO: use custom allocator
		 result->shp = res;
		 result->actor = nullptr;

		 shape = result;


		return true;
	}
	bool __CreateShapeWithTransform(void*& scene, void*& shape, trace::PhyShape& geometry, trace::Transform& transform, bool trigger)
	{
		if (shape)
		{
			TRC_WARN("Shape has already been created , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PxShape* res = nullptr;
		switch (geometry.type)
		{
		case trace::PhyShape::Box:
		{
			PxVec3 extents = { geometry.box.half_extents.x, geometry.box.half_extents.y, geometry.box.half_extents.z };
			res = phy.physics->createShape(PxBoxGeometry(extents), *phy.default_material);
			break;
		}
		case trace::PhyShape::Sphere:
		{
			float radius = geometry.sphere.radius;
			res = phy.physics->createShape(PxSphereGeometry(radius), *phy.default_material);
			break;
		}
		case trace::PhyShape::Capsule:
		{
			float radius = geometry.capsule.radius;
			float half_height = geometry.capsule.half_height;
			res = phy.physics->createShape(PxCapsuleGeometry(radius, half_height), *phy.default_material);
			break;
		}
		}



		if (!res)
		{
			TRC_ERROR("Unable to create Shape");
			return false;
		}

		if (trigger)
		{
			res->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			res->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);

		PxScene* scn = in_scene->scene;
		PxVec3 pos = Glm3ToPx3(transform.GetPosition());
		PxQuat orientation = GlmQuatToPxQuat(transform.GetRotation());
		PxTransform pose(pos, orientation);
		PxRigidStatic* act = phy.physics->createRigidStatic(pose);


		act->attachShape(*res);
		scn->addActor(*act);
		PhysxShape* result = new PhysxShape(); //TODO: use custom allocator
		result->shp = res;
		result->actor = act;
		result->scene = scn;

		shape = result;

		return true;
	}
	bool __UpdateShapeTransform(void*& shape, trace::Transform& transform)
	{
		
		if (!shape)
		{
			TRC_WARN("Enter a valid shape pointer , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PxVec3 pos = Glm3ToPx3(transform.GetPosition());
		PxQuat orientation = GlmQuatToPxQuat(transform.GetRotation());
		PxTransform pose(pos, orientation);

		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);
		if (res->actor)
		{
			res->actor->setGlobalPose(pose);
		}


		return true;
	}
	bool __DestroyShape(void*& shape)
	{
		if (!shape)
		{
			TRC_WARN("Enter a valid shape pointer , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);
		if (res->actor)
		{
			res->actor->detachShape(*res->shp);
			if (res->scene)
			{
				res->scene->removeActor(*res->actor);
			}
			PX_RELEASE(res->actor);
		}
		PX_RELEASE(res->shp);


		delete res; //TODO: use custom allocator
		shape = nullptr;

		return true;
	}
	bool __SetShapePtr(void*& shape, void* ptr)
	{

		if (!shape)
		{
			TRC_WARN("Enter a valid shape pointer , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);
		res->shp->userData = ptr;



		return true;
	}
	bool __SetShapeMask(void*& shape, uint32_t mask0, uint32_t mask1)
	{

		if (!shape)
		{
			TRC_WARN("Enter a valid shape pointer , ptr :{}, file : {}, line : {}", (const void*)shape, __FILE__, __LINE__);
			return false;
		}

		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);
		PxFilterData filterData;
		filterData.word0 = mask0;
		filterData.word1 = mask1;

		PxRigidActor* actor = res->actor;
		if (actor)
		{
			actor->detachShape(*res->shp);
		}

		res->shp->setSimulationFilterData(filterData);
		res->shp->setQueryFilterData(filterData);

		if (actor)
		{
			actor->attachShape(*res->shp);
		}

		return true;
	}
	bool __AttachShape(void*& shape, void*& actor)
	{
		if (!shape || !actor)
		{
			TRC_ERROR("Enter a valid shape or actor, file : {}, line : {}", __FILE__, __LINE__);
			return false;
		}

		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);
		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);

		if (res->actor)
		{
			res->actor->detachShape(*res->shp);
			if (res->scene)
			{
				res->scene->removeActor(*res->actor);
			}
			PX_RELEASE(res->actor);
		}

		body->attachShape(*res->shp);


		return true;
	}
	bool __DetachShape(void*& shape, void*& actor)
	{
		if (!shape || !actor)
		{
			TRC_ERROR("Enter a valid shape or actor, file : {}, line : {}", __FILE__, __LINE__);
			return false;
		}

		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);
		PhysxShape* res = reinterpret_cast<PhysxShape*>(shape);

		if (res->actor)
		{
			res->actor->detachShape(*res->shp);
			if (res->scene)
			{
				res->scene->removeActor(*res->actor);
			}
			PX_RELEASE(res->actor);
		}

		body->detachShape(*res->shp);


		return true;
	}
	bool __CreateRigidBody(trace::RigidBody& rigid_body, trace::Transform& transform)
	{
		void*& actor = rigid_body.GetInternal();
		if (actor)
		{
			TRC_WARN("rigid body has already been initialized , file : {}, line : {}", __FILE__, __LINE__);
			return false;
		}

		PxVec3 pos = Glm3ToPx3(transform.GetPosition());
		PxQuat orientation = GlmQuatToPxQuat(transform.GetRotation());
		PxTransform pose(pos, orientation);
		
		switch (rigid_body.GetType())
		{
		case trace::RigidBody::Static:
		{
			PxRigidStatic* res = phy.physics->createRigidStatic(pose);
			actor = res;
			break;
		}
		case trace::RigidBody::Dynamic:
		{
			PxRigidDynamic* res = phy.physics->createRigidDynamic(pose);
			actor = res;
			PxRigidBodyExt::updateMassAndInertia(*res, rigid_body.density);
			PxRigidBodyExt::setMassAndUpdateInertia(*res, rigid_body.mass);
			break;
		}
		case trace::RigidBody::Kinematic:
		{
			PxRigidDynamic* res = phy.physics->createRigidDynamic(pose);

			res->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			PxRigidBodyExt::updateMassAndInertia(*res, rigid_body.density);
			PxRigidBodyExt::setMassAndUpdateInertia(*res, rigid_body.mass);
			actor = res;
			break;
		}
		}

		if (!actor)
		{
			TRC_ERROR("Failed to create actor");
			return false;
		}

		return true;
	}
	bool __CreateRigidBody_Scene(void*& scene, trace::RigidBody& rigid_body, trace::Transform& transform)
	{
		if (__CreateRigidBody(rigid_body, transform))
		{
			PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);

			PxScene* scn = in_scene->scene;
			PxRigidActor* actor = reinterpret_cast<PxRigidActor*>(rigid_body.GetInternal());
			scn->addActor(*actor);
			return true;
		}
		return false;
	}
	bool __DestroyRigidBody(trace::RigidBody& rigid_body)
	{
		void*& actor = rigid_body.GetInternal();
		if (!actor)
		{
			TRC_WARN("Actor has already been destroyed");
			return false;
		}

		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);
		PX_RELEASE(body);
		actor = nullptr;
		

		return true;
	}
	bool __SetRigidBodyTransform(trace::RigidBody& rigid_body, trace::Transform& transform)
	{
		void*& actor = rigid_body.GetInternal();
		if (!actor)
		{
			TRC_WARN("Actor has already been destroyed");
			return false;
		}

		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);
		PxVec3 pos = Glm3ToPx3(transform.GetPosition());
		PxQuat orientation = GlmQuatToPxQuat(transform.GetRotation());
		PxTransform pose(pos, orientation);
		body->setGlobalPose(pose);

		return true;
	}
	bool __GetRigidBodyTransform(trace::RigidBody& rigid_body, trace::Transform& transform)
	{
		void*& actor = rigid_body.GetInternal();
		if (!actor)
		{
			TRC_WARN("Actor has already been destroyed");
			return false;
		}

		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);
		PxTransform pose = body->getGlobalPose();
		transform.SetPosition(Px3ToGlm3(pose.p));
		transform.SetRotation(PxQuatToGlmQuat(pose.q));

		return true;
	}
}