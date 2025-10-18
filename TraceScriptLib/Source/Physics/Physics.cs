using System;
using System.Runtime.InteropServices;



namespace Trace
{

    public enum ForceType
    {
        FORCE,				
		IMPULSE,			
		VELOCITY_CHANGE,	
		ACCELERATION
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct TriggerPair
    {
        public UInt64 entity;
        public UInt64 otherEntity;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct ContactPoint
    {
        public Vec3 point;
        public Vec3 normal;
        public float seperation;
    };

    [StructLayout(LayoutKind.Sequential)]
	public struct CollisionData
	{
        public UInt64 entity;
        public UInt64 otherEntity;
        public Vec3 impulse;
        public UInt32 numContacts;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]//NOTE: Always sync COLLISION_MAX_CONTACT_POINTS with SizeConst
        public ContactPoint[] contacts;
	};

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public UInt64 entity;
        public Vec3 position;
        public Vec3 normal;
        public float distance;
    };

    public class Physics
    {

        static public void Step(float deltaTime)
        {
            InternalCalls.Physics_Step(deltaTime);
        }

        public static void OnCollisionEnter(Trace.Action entity, Int64 collision_data)
        {
            CollisionData collision = new CollisionData();
            InternalCalls.Physics_GetCollisionData(ref collision, collision_data);
            entity.OnCollisionEnter(collision);
        }

        public static void OnCollisionExit(Trace.Action entity, Int64 collision_data)
        {
            CollisionData collision = new CollisionData();
            InternalCalls.Physics_GetCollisionData(ref collision, collision_data);
            entity.OnCollisionExit(collision);
        }

        public static void OnTriggerEnter(Trace.Action entity, Int64 trigger_data)
        {
            TriggerPair trigger = new TriggerPair();
            InternalCalls.Physics_GetTriggerData(ref trigger, trigger_data);
            entity.OnTriggerEnter(trigger);
        }

        public static void OnTriggerExit(Trace.Action entity, Int64 trigger_data)
        {
            TriggerPair trigger = new TriggerPair();
            InternalCalls.Physics_GetTriggerData(ref trigger, trigger_data);
            entity.OnTriggerExit(trigger);
        }

        public static bool RayCast(Vec3 origin, Vec3 direction, float max_distance, out RaycastHit result)
        {
            return InternalCalls.Physics_RayCast(ref origin, ref direction, max_distance, out result);
        }



    };

}
