using System;
using System.Runtime.CompilerServices;

namespace Trace
{
    internal static class InternalCalls
    {

        #region Actions
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Action_HasComponent(ulong id, Type component_type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Action_HasScript(ulong id, Type script_type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Action_RemoveComponent(ulong id, Type component_type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Action_RemoveScript(ulong id, Type script_type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Action_GetComponent(ulong id, Type component_type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Action_GetScript(ulong id, Type script_type);
        #endregion


        #region Debug
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Log(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Trace(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Warn(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Error(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Info(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Debug_Critical(string message);
        #endregion

    }
}
