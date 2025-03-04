using System;
using System.Runtime.InteropServices;



namespace Trace
{

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

    public class Physics
    {

        public static void OnCollisionEnter(Trace.Action entity, Int64 collision_data)
        {
            CollisionData collision = new CollisionData();
            InternalCalls.Physics_GetCollisionData(ref collision, collision_data);
            entity.OnCollisionEnter(collision);
        }

    };

}
