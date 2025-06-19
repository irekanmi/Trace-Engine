using System;
using System.Collections.Generic;

namespace Trace
{
    public enum RPCType
    {
        UNKNOW,
        CLIENT,
        SERVER
    }

    static public class Network
    {
        public static event Action<uint> on_client_connect;
        public static event Action<uint> on_client_disconnect;
        public static event Action<uint> on_server_connect;
        public static event Action<uint> on_server_disconnect;

        static void OnClientConnect(uint handle)
        {
            on_client_connect?.Invoke(handle);
        }
        static void OnClientDisconnect(uint handle)
        {
            on_client_disconnect?.Invoke(handle);
        }
        static void OnServerConnect(uint handle)
        {
            on_server_connect?.Invoke(handle);
        }
        
        static void OnServerDisconnect(uint handle)
        {
           on_server_disconnect?.Invoke(handle);
        }

        static void ReleaseEventObject()
        {
            on_client_connect = null;
            on_client_disconnect = null;
            on_server_connect = null;
            on_server_disconnect = null;
        }

        public static bool IsServer()
        {
            return InternalCalls.Networking_IsServer();
        }

        public static bool IsClient()
        {
            return InternalCalls.Networking_IsClient();
        }

        public static void InvokeRPC(Trace.Action src, UInt64 func_name_id, RPCType type)// TODO: Allow for Parameters
        {
            InternalCalls.Networking_InvokeRPC( src.GetID(), src, func_name_id, type);
        }
    }
}
