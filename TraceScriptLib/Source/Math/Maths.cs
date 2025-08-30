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
        
        static public Vec2 operator/(Vec2 a, float b)
        {
            return new Vec2 { x = a.x / b, y = a.y / b };
        }

        static public Vec2 operator +(Vec2 a, Vec2 b)
        {
            return new Vec2 { x = a.x * b.x, y = a.y * b.y };
        }

        static public bool operator ==(Vec2 a, Vec2 b)
        {
            return a.x == b.x && a.y == b.y;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override string ToString()
        {
            return "X: " + x.ToString() + " Y: " + y.ToString();
        }

        static public bool operator !=(Vec2 a, Vec2 b)
        {
            return !(a == b);
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
        
        static public Vec3 operator/(Vec3 a, float b)
        {
            return new Vec3 { x = a.x / b, y = a.y / b, z = a.z / b };
        }


        static public Vec3 operator +(Vec3 a, Vec3 b)
        {
            return new Vec3 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z };
        }

        static public Vec3 operator -(Vec3 a, Vec3 b)
        {
            return new Vec3 { x = a.x - b.x, y = a.y - b.y, z = a.z - b.z };
        }

        static public bool operator ==(Vec3 a, Vec3 b)
        {
            return a.x == b.x && a.y == b.y && a.z == b.z;
        }

        static public bool operator !=(Vec3 a, Vec3 b)
        {
            return !(a == b);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override string ToString()
        {
            return "X: " + x.ToString() + " Y: " + y.ToString() + " Z: " + z.ToString();
        }


    }

    public struct Quat
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public Quat(float _x, float _y, float _z, float _w)
        {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
        }

        public Quat(Vec3 euler_angles)
        {
            InternalCalls.Maths_Quat_Set_Euler_Angle(out Quat result, ref euler_angles);
            this = result;
        }

        public Vec3 EulerAngle
        {
            get
            {
                InternalCalls.Maths_Quat_Get_Euler_Angle( ref this, out Vec3 result);
                return result;
            }

            set
            {
                InternalCalls.Maths_Quat_Set_Euler_Angle(out Quat result, ref value);
                this = result;
            }
        }

        public static Quat Identity => new Quat { x = 0, y = 0, z = 0, w = 1 };


        static public Vec3 operator *(Quat a, Vec3 b)
        {
            InternalCalls.Maths_Quat_Mul_Vec(ref a, ref b, out Vec3 result);
            return result;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override string ToString()
        {
            return "X: " + x.ToString() + " Y: " + y.ToString() + " Z: " + z.ToString() + " W: " + w.ToString();
        }

    }



    public static class Maths
    {

        static public Vec2 Lerp(Vec2 a, Vec2 b, float t)
        {
            return new Vec2(Utils.Lerp(a.x, b.x, t), Utils.Lerp(a.y, b.y, t));
        }

        static public Vec3 Lerp(Vec3 a, Vec3 b, float t)
        {
            return new Vec3(Utils.Lerp(a.x, b.x, t), Utils.Lerp(a.y, b.y, t), Utils.Lerp(a.z, b.z, t));
        }


        public static Quat Slerp(Quat a, Quat b, float t)
        {
            InternalCalls.Maths_Quat_Slerp(ref a, ref b, t, out Quat result);
            return result;
        }
        public static Quat LookDirection(Vec3 direction)
        {
            InternalCalls.Maths_Quat_LookDirection(ref direction, out Quat result);
            return result;
        }

        static public float Length2(Vec2 val)
        {
            float result = (val.x * val.x) + (val.y * val.y);
            return result;
        }
        static public float Length2(Vec3 val)
        {
            float result = (val.x * val.x) + (val.y * val.y) + (val.z * val.z);
            return result;
        }
        
        static public float Length(Vec2 val)
        {
            return (float)Math.Sqrt((double)Length2(val));
        }
        static public float Length(Vec3 val)
        {
            return (float)Math.Sqrt((double)Length2(val));
        }
        
        static public Vec2 Normalize(Vec2 val)
        {
            return val / Length(val);
        }
        
        static public Vec3 Normalize(Vec3 val)
        {
            return val / Length(val);
        }

    }



}