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

        #region TransformComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_GetPosition(ulong id, out Vec3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_SetPosition(ulong id, ref Vec3 position);

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

        #region Input

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetKey(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetButton(Buttons buttons);

        #endregion

        #region TextComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static string TextComponent_GetString(ulong id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TextComponent_SetString(ulong id, string text);

        #endregion

    }
}
