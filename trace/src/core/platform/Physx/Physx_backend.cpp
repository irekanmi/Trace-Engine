#include "pch.h"

#include "Physx_backend.h"
#include "core/io/Logging.h"
#include "glm/gtc/quaternion.hpp"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBackend.h"
#include "multithreading/JobSystem.h"


// HACK: Fix "Physx\foundation\PxPreprocessor.h(443,1): fatal error C1189: #error:  Exactly one of NDEBUG and _DEBUG needs to be defined!"
#ifdef TRC_DEBUG_BUILD
#define NDEBUG
#else
#define NDEBUG
#endif
//
#include "PxPhysicsAPI.h"


namespace physx {
	static glm::vec3 Px3ToGlm3(PxVec3& val);
	static trace::Counter* job_counter = nullptr;
	// physx doesn't allow rigid actor creation or modification while stimulation is running, so this lock prevents that
	static trace::SpinLock stimulation_lock;
	

	class JobDispatcher : public PxCpuDispatcher
	{
	public:
		void submitTask(PxBaseTask& task) override
		{
			trace::Job job;
			job.flags = trace::JobFlagBit::GENERAL;
			job.job_func = [&task](void*)
			{
				task.run();
				task.release();
			};

			trace::JobSystem::get_instance()->RunJob(job, job_counter);
		}

		uint32_t getWorkerCount() const override
		{
			return trace::JobSystem::get_instance()->GetThreadCount();
		}
	};

	class SceneEventCallback : public PxSimulationEventCallback
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

			// Skip sending events to removed actors
			if (pairHeader.flags & (PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1))
			{
				return;
			}

			for (PxU32 i = 0; i < nbPairs; i++)
			{
				const PxContactPair& contact_pair = pairs[i];

				PxContactStreamIterator iter(contact_pair.contactPatches, contact_pair.contactPoints, contact_pair.getInternalFaceIndices(), contact_pair.patchCount, contact_pair.contactCount);

				const PxReal* impulses = contact_pair.contactImpulses;

				PxU32 flippedContacts = (contact_pair.flags & PxContactPairFlag::eINTERNAL_CONTACTS_ARE_FLIPPED);
				PxU32 hasImpulses = (contact_pair.flags & PxContactPairFlag::eINTERNAL_HAS_IMPULSES);
				PxU32 nbContacts = 0;

				Entity entity = *(Entity*)contact_pair.shapes[0]->userData;
				Entity otherEntity = *(Entity*)contact_pair.shapes[1]->userData;

				trace::CollisionData collision;
				collision.entity = entity.GetID();
				collision.otherEntity = otherEntity.GetID();

				while (iter.hasNextPatch())
				{
					iter.nextPatch();
					while (iter.hasNextContact())
					{
						iter.nextContact();
						PxVec3 point = iter.getContactPoint();
						PxVec3 normal = iter.getContactNormal();
						PxVec3 impulse = hasImpulses ? normal * impulses[nbContacts] : PxVec3(0.f);

						PxU32 internalFaceIndex0 = flippedContacts ?
							iter.getFaceIndex1() : iter.getFaceIndex0();
						PxU32 internalFaceIndex1 = flippedContacts ?
							iter.getFaceIndex0() : iter.getFaceIndex1();
						//...

						trace::ContactPoint contact_point = {};
						contact_point.point = Px3ToGlm3(point);
						contact_point.normal = Px3ToGlm3(normal);
						contact_point.seperation = iter.getSeparation();

						collision.contacts[nbContacts] = contact_point;

						nbContacts++;
					}
				}

				collision.numContacts = nbContacts;


				trace::CollisionPair collision_pair = std::make_pair(collision.entity, collision.otherEntity);
				if (contact_pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
				{
					StayCollisions.push_back(collision_pair);
				}
				CollisionsMap[collision_pair] = collision;
			}

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

				trace::TriggerPair trigger_pair = {};
				trigger_pair.entity = trigger->GetID();
				trigger_pair.otherEntity = otherCollider->GetID();


				if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					ExitTriggers.emplace_back(trigger_pair);
				}
				else
				{
					EnterTriggers.emplace_back(trigger_pair);
				}
			}
		}

		virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
		{

		}

		void SendTriggerEvents()
		{
			// Temp __________________

			trace::ScriptEngine* script_engine = trace::ScriptEngine::get_instance();
			trace::Scene* scene = script_engine->GetCurrentScene();
			trace::ScriptRegistry& scene_registry = scene->GetScriptRegistry();

			for (auto& i : EnterTriggers)
			{				

				scene_registry.Iterate(i.entity, [&i](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_trigger_enter = script->GetMethod("OnTriggerEnter");
						if (!on_trigger_enter) return;

						Script_OnTriggerEnter(i.entity, *instance, i);
					});
				i.Swap();
				scene_registry.Iterate(i.entity, [&i](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_trigger_enter = script->GetMethod("OnTriggerEnter");
						if (!on_trigger_enter) return;

						Script_OnTriggerEnter(i.entity, *instance, i);
					});

				
			}

			for (auto& i : ExitTriggers)
			{

				scene_registry.Iterate(i.entity, [&i](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_trigger_exit = script->GetMethod("OnTriggerExit");
						if (!on_trigger_exit) return;

						Script_OnTriggerExit(i.entity, *instance, i);
					});
				i.Swap();
				scene_registry.Iterate(i.entity, [&i](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_trigger_exit = script->GetMethod("OnTriggerExit");
						if (!on_trigger_exit) return;

						Script_OnTriggerExit(i.entity, *instance, i);
					});
			}

			// _______________________
		}

		void SendCollisionEvents()
		{
			trace::ScriptEngine* script_engine = trace::ScriptEngine::get_instance();
			trace::Scene* scene = script_engine->GetCurrentScene();
			trace::ScriptRegistry& scene_registry = scene->GetScriptRegistry();

			for (auto& i : NewCollisions)
			{
				trace::CollisionData& col = CollisionsMap[i];

				scene_registry.Iterate(col.entity, [&col](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_collision_enter = script->GetMethod("OnCollisionEnter");
						if (!on_collision_enter)
						{
							return;
						}

						Script_OnCollisionEnter(uuid, *instance, col);
					});

				col.Swap();

				scene_registry.Iterate(col.entity, [&col](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_collision_enter = script->GetMethod("OnCollisionEnter");
						if (!on_collision_enter)
						{
							return;
						}

						Script_OnCollisionEnter(uuid, *instance, col);
					});
			}

			for (auto& i : StayCollisions)
			{
				trace::CollisionData& col = CollisionsMap[i];

			}

			for (auto& i : RemovedCollisions)
			{
				trace::CollisionData& col = PrevCollisions[i];


				scene_registry.Iterate(col.entity, [&col](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_collision_exit = script->GetMethod("OnCollisionExit");
						if (!on_collision_exit)
						{
							return;
						}

						Script_OnCollisionExit(uuid, *instance, col);
					});

				col.Swap();

				scene_registry.Iterate(col.entity, [&col](UUID uuid, Script* script, ScriptInstance* instance)
					{
						ScriptMethod* on_collision_exit = script->GetMethod("OnCollisionExit");
						if (!on_collision_exit)
						{
							return;
						}

						Script_OnCollisionExit(uuid, *instance, col);
					});

			}

		}

		void SendEvents()
		{

			SendTriggerEvents();
			SendCollisionEvents();
			
		}

		void Clear()
		{
			PrevCollisions = CollisionsMap;
			CollisionsMap.clear();

			NewCollisions.clear();
			RemovedCollisions.clear();
			StayCollisions.clear();


			EnterTriggers.clear();
			ExitTriggers.clear();
		}

		void CollectEvents()
		{
			for (auto& i : CollisionsMap)
			{
				auto prev = PrevCollisions.find(i.first);
				if (prev == PrevCollisions.end())
					NewCollisions.push_back(i.first);
			}

			for (auto& i : PrevCollisions)
			{
				auto it = CollisionsMap.find(i.first);
				if (it == CollisionsMap.end())
					RemovedCollisions.push_back(i.first);
			}
		}

	private:
		std::vector<trace::TriggerPair> EnterTriggers;
		std::vector<trace::TriggerPair> ExitTriggers;

		std::unordered_map<trace::CollisionPair, CollisionData> PrevCollisions;
		std::vector<trace::CollisionPair> NewCollisions;
		std::vector<trace::CollisionPair> RemovedCollisions;
		std::vector<trace::CollisionPair> StayCollisions;
		std::unordered_map<trace::CollisionPair, CollisionData> CollisionsMap;


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
		PxCpuDispatcher* cpu_dispatcher = nullptr;
		PxPvd* pvd = nullptr;
		Physx_ErrorCallback error_cb;
		PxDefaultAllocator allocator;
		PxMaterial* default_material;
	};

	struct PhysxScene
	{
		PxScene* scene = nullptr;
		SceneEventCallback event_callback;
		PxControllerManager* controller_manager = nullptr;
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
			pairFlags |= PxPairFlag::eSOLVE_CONTACT; //TODO: Check Docks to understand Flag
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
		phy.cpu_dispatcher = new JobDispatcher;//TODO: Use custom allocator

		job_counter = trace::JobSystem::get_instance()->CreateCounter();

		return true;
	}

	bool __ShutdownPhysics3D()
	{
		trace::JobSystem::get_instance()->WaitForCounterAndFree(job_counter);

		PX_RELEASE(phy.default_material);
		//PX_RELEASE(phy.cpu_dispatcher);
		delete phy.cpu_dispatcher;//TODO: Use custom allocator
		phy.cpu_dispatcher = nullptr;
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
		//desc.flags = PxSceneFlag::eREQUIRE_RW_LOCK;

		PxScene* res = phy.physics->createScene(desc);
		if (res)
		{		
			out_scene->scene = res;
			scene = out_scene;
			out_scene->controller_manager = PxCreateControllerManager(*res);
		}
		else
		{
			TRC_ASSERT(false, "Unable to Create Physx scene, Funtion: {}", __FUNCTION__);
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
		PxControllerManager* manager = in_scene->controller_manager;
		PX_RELEASE(manager);
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

		stimulation_lock.Lock();

		scn->simulate(deltaTime);
		trace::JobSystem::get_instance()->WaitForCounter(job_counter);
		scn->fetchResults(true);
		trace::JobSystem::get_instance()->WaitForCounter(job_counter);

		stimulation_lock.Unlock();


		in_scene->event_callback.CollectEvents();
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
		case trace::PhyShapeType::Box:
		{
			PxVec3 extents = { geometry.box.half_extents.x, geometry.box.half_extents.y, geometry.box.half_extents.z };
			res = phy.physics->createShape(PxBoxGeometry(extents), *phy.default_material);
			break;
		}
		case trace::PhyShapeType::Sphere:
		{
			float radius = geometry.sphere.radius;
			res = phy.physics->createShape(PxSphereGeometry(radius), *phy.default_material);
			break;
		}
		case trace::PhyShapeType::Capsule:
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
		case trace::PhyShapeType::Box:
		{
			PxVec3 extents = { geometry.box.half_extents.x, geometry.box.half_extents.y, geometry.box.half_extents.z };
			res = phy.physics->createShape(PxBoxGeometry(extents), *phy.default_material);
			break;
		}
		case trace::PhyShapeType::Sphere:
		{
			float radius = geometry.sphere.radius;
			res = phy.physics->createShape(PxSphereGeometry(radius), *phy.default_material);
			break;
		}
		case trace::PhyShapeType::Capsule:
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
		scn->lockWrite();
		scn->addActor(*act);
		scn->unlockWrite();
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
		orientation.normalize();
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
		case trace::RigidBodyType::Static:
		{
			PxRigidStatic* res = phy.physics->createRigidStatic(pose);
			actor = res;
			break;
		}
		case trace::RigidBodyType::Dynamic:
		{
			PxRigidDynamic* res = phy.physics->createRigidDynamic(pose);
			actor = res;
			PxRigidBodyExt::updateMassAndInertia(*res, rigid_body.density);
			PxRigidBodyExt::setMassAndUpdateInertia(*res, rigid_body.mass);
			break;
		}
		case trace::RigidBodyType::Kinematic:
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
			scn->lockWrite();
			scn->addActor(*actor);
			scn->unlockWrite();
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

		stimulation_lock.Lock();
		body->getScene()->lockWrite();
		body->setGlobalPose(pose);
		body->getScene()->unlockWrite();
		stimulation_lock.Unlock();

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

		stimulation_lock.Lock();
		body->getScene()->lockRead();
		PxTransform pose = body->getGlobalPose();
		body->getScene()->unlockRead();
		stimulation_lock.Unlock();

		transform.SetPosition(Px3ToGlm3(pose.p));
		transform.SetRotation(PxQuatToGlmQuat(pose.q));

		return true;
	}
	bool __CreateCharacterController(trace::CharacterController& controller, void*& scene, trace::Transform& pose)
	{
		bool result = true;

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);

		const float min_contact_offset = 0.00000001f;

		PxCapsuleControllerDesc desc = {};
		desc.behaviorCallback = nullptr;
		desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		desc.contactOffset = controller.contact_offset;
		desc.contactOffset = glm::max(desc.contactOffset, min_contact_offset);
		desc.height = controller.height;
		desc.material = phy.default_material;
		desc.stepOffset = controller.step_offset;
		desc.radius = controller.radius;
		desc.slopeLimit = cosf(glm::radians(controller.slope_limit));
		glm::vec3 pos = pose.GetPosition();
		pos += controller.offset;
		desc.position = PxExtendedVec3(pos.x, pos.y, pos.z);
		desc.reportCallback = nullptr;

		PxController* res = nullptr;

		stimulation_lock.Lock();
		res = in_scene->controller_manager->createController(desc);
		stimulation_lock.Unlock();

		TRC_ASSERT(res != nullptr, "Unable to Create Character Controller, Function: {}", __FUNCTION__);

		controller.SetInternal(res);

		return result;
	}
	bool __DestroyCharacterController(trace::CharacterController& controller, void*& scene)
	{
		PxController* res = reinterpret_cast<PxController*>(controller.GetInternal());
		
		TRC_ASSERT(res != nullptr, "Invalid Character Controller, Function: {}", __FUNCTION__);

		PX_RELEASE(res);

		return true;
	}
	bool __MoveCharacterController(trace::CharacterController& controller, glm::vec3 displacement, float deltaTime)
	{

		PxController* res = reinterpret_cast<PxController*>(controller.GetInternal());

		TRC_ASSERT(res != nullptr, "Invalid Character Controller, Function: {}", __FUNCTION__);

		stimulation_lock.Lock();
		res->getScene()->lockRead();
		PxControllerCollisionFlags collisionFlags = res->move( Glm3ToPx3(displacement), controller.min_move_distance, deltaTime, PxControllerFilters());
		res->getScene()->unlockRead();
		stimulation_lock.Unlock();

		bool is_down = TRC_HAS_FLAG(collisionFlags, PxControllerCollisionFlag::eCOLLISION_DOWN);

		controller.SetIsGrounded(is_down);

		return true;
	}
	bool __SetControllerDataPtr(trace::CharacterController& controller, void* ptr)
	{
		PxController* res = reinterpret_cast<PxController*>(controller.GetInternal());

		TRC_ASSERT(res != nullptr, "Invalid Character Controller, Function: {}", __FUNCTION__);

		PxRigidActor* actor = res->getActor();
		PxShape* shape = nullptr;
		actor->getShapes(&shape, 1);
		
		TRC_ASSERT(shape != nullptr, "Unable to get Character Controller shape, Function: {}", __FUNCTION__);

		shape->userData = ptr;

		return true;
	}
	bool __GetCharacterControllerPosition(trace::CharacterController& controller, glm::vec3& out_position)
	{
		PxController* res = reinterpret_cast<PxController*>(controller.GetInternal());

		TRC_ASSERT(res != nullptr, "Invalid Character Controller, Function: {}", __FUNCTION__);

		PxExtendedVec3 pos = res->getPosition();
		out_position.x = static_cast<float>(pos.x) - controller.offset.x;
		out_position.y = static_cast<float>(pos.y) - controller.offset.y;
		out_position.z = static_cast<float>(pos.z) - controller.offset.z;

		return true;
	}
	
	bool __SetCharacterControllerPosition(trace::CharacterController& controller, glm::vec3& position)
	{
		PxController* res = reinterpret_cast<PxController*>(controller.GetInternal());

		TRC_ASSERT(res != nullptr, "Invalid Character Controller, Function: {}", __FUNCTION__);

		PxExtendedVec3 pos;
		pos.x = static_cast<float>(position.x) + controller.offset.x;
		pos.y = static_cast<float>(position.y) + controller.offset.y;
		pos.z = static_cast<float>(position.z) + controller.offset.z;

		stimulation_lock.Lock();
		res->getScene()->lockWrite();
		res->setPosition(pos);
		res->getScene()->unlockWrite();
		stimulation_lock.Unlock();
		/*PxControllerCollisionFlags collisionFlags = res->move(Glm3ToPx3(glm::vec3(0.0f)), controller.min_move_distance, 0.0f, PxControllerFilters());

		bool is_down = TRC_HAS_FLAG(collisionFlags, PxControllerCollisionFlag::eCOLLISION_DOWN);

		controller.SetIsGrounded(is_down);*/

		return true;
	}
	bool __RayCast(void* scene, glm::vec3 origin, glm::vec3 direction, float max_distance, trace::RaycastHit& result)
	{

		TRC_ASSERT(scene, "This pointer is invaild");

		PhysxScene* in_scene = reinterpret_cast<PhysxScene*>(scene);
		PxRaycastHit ray_hit;
		PxRaycastBuffer buf(&ray_hit, 1);

		bool hit_result = in_scene->scene->raycast(Glm3ToPx3(origin), Glm3ToPx3(direction), max_distance, buf);

		if (hit_result)
		{
			result.position = Px3ToGlm3(ray_hit.position);
			result.normal = Px3ToGlm3(ray_hit.normal);
			result.distance = ray_hit.distance;
			TRC_ASSERT(ray_hit.shape->userData, "Entity ptr should be assigned");
			trace::Entity* entity = (trace::Entity*)ray_hit.shape->userData;
			result.entity = entity->GetID();
		}

		return hit_result;
	}
}