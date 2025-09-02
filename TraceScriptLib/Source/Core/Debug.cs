using System;
using System.Collections.Generic;
using System.Linq;


namespace Trace
{
    static public class Debug
    {
        public static void Log(string message)
        {
            InternalCalls.Debug_Log(message);
        }

        public static void Trace(string message)
        {
            InternalCalls.Debug_Trace(message);
        }

        public static void Info(string message)
        {
            InternalCalls.Debug_Info(message);
        }

        public static void Sphere(Vec3 position, Vec3? color = null, float radius = 2.5f, UInt32 steps = 7)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 1.0f, 0.0f);

            InternalCalls.Debug_Sphere(ref position, radius, steps, ref final_color);
        }
        
        
        public static void Sphere(Vec3 position, float duration, Vec3? color = null, float radius = 2.5f, UInt32 steps = 7)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 1.0f, 0.0f);

            InternalCalls.Debug_SphereTimed(duration, ref position, radius, steps, ref final_color);
        }
        public static void Line(Vec3 from, Vec3 to, Vec3? color = null)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 0.4f, 0.7f);

            InternalCalls.Debug_Line(ref from, ref to, ref final_color);
        }
        
        public static void Line(Vec3 from, Vec3 to, float duration, Vec3? color = null)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 0.4f, 0.7f);

            InternalCalls.Debug_LineTimed(duration, ref from, ref to, ref final_color);
        }
        
        public static void Box(Vec3 half_extents, Mat4 transform, Vec3? color = null)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 0.0f, 1.0f);

            InternalCalls.Debug_Box(ref half_extents, ref transform, ref final_color);
        }
        
        public static void Box(Vec3 half_extents, Mat4 transform, float duration, Vec3? color = null)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 0.0f, 1.0f);

            InternalCalls.Debug_BoxTimed(duration, ref half_extents, ref transform, ref final_color);
        }
        
        public static void Box(Vec3 half_extents, Vec3 position, float duration, Vec3? color = null)
        {
            Vec3 final_color = color ?? new Vec3(0.0f, 0.0f, 1.0f);

            Mat4 transform = new Mat4(1.0f);
            transform.row_3.XYZ = position;

            InternalCalls.Debug_BoxTimed(duration, ref half_extents, ref transform, ref final_color);
        }

    }
}
