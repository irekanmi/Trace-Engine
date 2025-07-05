using System;
using System.Collections.Generic;


namespace Trace
{
    public static class Stream
    {

        public static void WriteInt(UInt64 stream_handle, int value)
        {
            InternalCalls.Stream_WriteInt(stream_handle, value);
        }
        public static void WriteFloat(UInt64 stream_handle, float value)
        {
            InternalCalls.Stream_WriteFloat(stream_handle, value);
        }

        public static void WriteVec2(UInt64 stream_handle, Vec2 value)
        {
            InternalCalls.Stream_WriteVec2(stream_handle, ref value);
        }

        public static void WriteVec3(UInt64 stream_handle, Vec3 value)
        {
            InternalCalls.Stream_WriteVec3(stream_handle, ref value);
        }
        
        public static void WriteQuat(UInt64 stream_handle, Quat value)
        {
            InternalCalls.Stream_WriteQuat(stream_handle, ref value);
        }

        public static int ReadInt(UInt64 stream_handle)
        {
            InternalCalls.Stream_ReadInt(stream_handle,out int value);
            return value;
        }

        public static float ReadFloat(UInt64 stream_handle)
        {
            InternalCalls.Stream_ReadFloat(stream_handle, out float value);
            return value;
        }

        public static Vec2 ReadVec2(UInt64 stream_handle)
        {
            InternalCalls.Stream_ReadVec2(stream_handle, out Vec2 value);
            return value;
        }
        public static Vec3 ReadVec3(UInt64 stream_handle)
        {
            InternalCalls.Stream_ReadVec3(stream_handle, out Vec3 value);
            return value;
        }
        public static Quat ReadQuat(UInt64 stream_handle)
        {
            InternalCalls.Stream_ReadQuat(stream_handle, out Quat value);
            return value;
        }


    }
}
