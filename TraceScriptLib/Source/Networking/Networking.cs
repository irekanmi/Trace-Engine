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
        // static Event on_client_connect
        // static Event on_client_disconnect
        // static Event on_server_connect
        // static Event on_server_disconnect

        static void OnClientConnect(uint handle)
        {
            // Event on_client_connect()
        }
        static void OnClientDisconnect(uint handle)
        {
            // Event on_client_disconnect()
        }
        static void OnServerConnect(uint handle)
        {
            // Event on_server_connect()
        }
        
        static void OnServerDisconnect(uint handle)
        {
            // Event on_server_disconnect()
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
