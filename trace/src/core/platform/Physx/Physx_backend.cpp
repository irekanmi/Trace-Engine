#include "pch.h"

#include "Physx_backend.h"
#include "core/io/Logging.h"
#include "glm/gtc/quaternion.hpp"


// HACK: Fix "Physx\foundation\PxPreprocessor.h(443,1): fatal error C1189: #error:  Exactly one of NDEBUG and _DEBUG needs to be defined!"
#ifdef TRC_DEBUG_BUILD
#define NDEBUG
#else
#define NDEBUG
#endif
//
#include "PxPhysicsAPI.h"


namespace physx {

	struct PhysxScene
	{
		PxScene* _internal;
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
		desc.cpuDispatcher = phy.cpu_dispatcher;
		desc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		desc.filterShader = PxDefaultSimulationFilterShader;

		PxScene* res = phy.physics->createScene(desc);
		if (res)
		{			
			scene = res;
		}
		else
		{
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

		PhysxScene* res = reinterpret_cast<PhysxScene*>(scene);
		PX_RELEASE(res->_internal);
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

		PxScene* scn = reinterpret_cast<PxScene*>(scene);
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

		PxScene* scn = reinterpret_cast<PxScene*>(scn);
		PxRigidActor* body = reinterpret_cast<PxRigidActor*>(actor);

		scn->removeActor(*body);

		return true;
	}
	bool __Stimulate(void*& scene, float deltaTime)
	{
		PxScene* scn = reinterpret_cast<PxScene*>(scene);
		scn->simulate(deltaTime);
		scn->fetchResults(true);
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
			res->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		shape = res;

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
		PxShape* shp = reinterpret_cast<PxShape*>(shape);

		body->attachShape(*shp);


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
		PxShape* shp = reinterpret_cast<PxShape*>(shape);

		body->detachShape(*shp);


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