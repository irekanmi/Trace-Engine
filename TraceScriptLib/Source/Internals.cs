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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static string Action_GetName(ulong id);
        #endregion

        #region TransformComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_GetPosition(ulong id, out Vec3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_SetPosition(ulong id, ref Vec3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_GetWorldPosition(ulong id, out Vec3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_SetWorldPosition(ulong id, ref Vec3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_GetRotation(ulong id, out Quat rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TransformComponent_SetRotation(ulong id, ref Quat rotation);

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
        extern public static bool Input_GetKeyPressed(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetKeyReleased(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetButton(Buttons buttons);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetButtonPressed(Buttons buttons);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool Input_GetButtonReleased(Buttons buttons);

        #endregion

        #region TextComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static string TextComponent_GetString(ulong id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void TextComponent_SetString(ulong id, string text);

        #endregion

        #region Scene

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Scene_GetEntityByName(ulong id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Scene_GetEntity(ulong entity_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Scene_GetChildEntityByName(ulong id, ulong child_name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static object Scene_InstanciateEntity_Position(ulong id, ref Vec3 position);

        #endregion

        #region Physics

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Physics_GetCollisionData(ref CollisionData data, Int64 collision_data);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Physics_GetTriggerData(ref TriggerPair data, Int64 trigger_data);

        #endregion

        #region CharacterController

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static bool CharacterController_IsGrounded(ulong entity_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void CharacterController_Move(ulong entity_id, ref Vec3 displacement, float deltaTime);

        #endregion

        #region AnimationGraphController

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void AnimationGraphController_SetParameterBool(ulong entity_id,string parameter_name, bool value);

        #endregion

        #region Maths

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Maths_Quat_LookDirection(ref Vec3 direction, out Quat result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Maths_Quat_Slerp(ref Quat a, ref Quat b, float lerp_value, out Quat out_rotation);


        #endregion

    }
}
