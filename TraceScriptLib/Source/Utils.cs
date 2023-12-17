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

        public static Vec2 Zero
        {
            get
            {
                return new Vec2(0.0f, 0.0f);
            }
            private set
            {
                return;
            }
        }

        public float x;
        public float y;

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

        public static Vec3 Zero
        {
            get
            {
                return new Vec3(0.0f, 0.0f, 0.0f);
            }
            private set
            {
                return;
            }
        }

        public float x;
        public float y;
        public float z;

    }

    public struct Collision
    {
        public Vec3 position;
        public ulong entity;
        public Action other;
    };
    

}