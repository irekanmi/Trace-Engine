using System;

namespace Trace
{

    public struct Vec2
    {
        public Vec2(float _x, float _y)
        {
            x = _x;
            y = _y;
        }

        public Vec2(Vec2 other)
        {
            x = other.x;
            y = other.y;
        }

        public Vec2 YX
        {
            get
            {
                return new Vec2(y, x);
            }
            set
            {
                y = value.x;
                x = value.y;
            }
        }

        public static Vec2 Zero => new Vec2 { x = 0, y = 0 };

        public float x;
        public float y;


        static public Vec2 operator*(Vec2 a, float b)
        {
            return new Vec2 { x = a.x * b, y = a.y * b };
        }

        static public Vec2 operator +(Vec2 a, Vec2 b)
        {
            return new Vec2 { x = a.x * b.x, y = a.y * b.y };
        }

    }

    public struct Vec3
    {

        public Vec3(float _x, float _y, float _z)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        public Vec3(Vec2 _xy, float _z)
        {
            x = _xy.x;
            y = _xy.y;
            z = _z;
        }

        public Vec3(Vec3 other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        public Vec2 YX
        {
            get
            {
                return new Vec2(y, x);
            }
            set
            {
                y = value.x;
                x = value.y;
            }
        }

        public Vec2 XY
        {
            get
            {
                return new Vec2(x, y);
            }
            set
            {
                x = value.x;
                y = value.y;
            }
        }
        public Vec2 XZ
        {
            get
            {
                return new Vec2(x, z);
            }
            set
            {
                x = value.x;
                z = value.y;
            }
        }

        public Vec2 YZ
        {
            get
            {
                return new Vec2(y, z);
            }
            set
            {
                y = value.x;
                z = value.y;
            }
        }

        public static Vec3 Zero => new Vec3 { x = 0, y = 0, z = 0 };

        public float x;
        public float y;
        public float z;


        static public Vec3 operator*(Vec3 a, float b)
        {
            return new Vec3 { x = a.x * b, y = a.y * b, z = a.z * b };
        }


        static public Vec3 operator +(Vec3 a, Vec3 b)
        {
            return new Vec3 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z };
        }


    }

    /// <summary>
    /// triggerEntity - Entity which holds the trigger
    /// otherEntity - Entity that entered or exited the trigger
    /// </summary>
    public class TriggerPair
    {
        public TriggerPair() { triggerEntity = null; otherEntity = null; }
        public Action triggerEntity;
        public Action otherEntity;
    };
    

}