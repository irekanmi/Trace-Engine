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

    }
}
