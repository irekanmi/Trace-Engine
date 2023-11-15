#include "pch.h"

#include "Physx_backend.h"
#include "core/io/Logging.h"


// HACK: Fix "Physx\foundation\PxPreprocessor.h(443,1): fatal error C1189: #error:  Exactly one of NDEBUG and _DEBUG needs to be defined!"
#ifdef TRC_DEBUG_BUILD
#define NDEBUG
#else
#define NDEBUG
#endif
//
#include "PxPhysicsAPI.h"


namespace physx {




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
}