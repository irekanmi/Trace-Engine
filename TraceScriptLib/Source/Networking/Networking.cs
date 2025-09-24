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

        readonly public static uint DEAFAULT_SERVER_PORT = 2367;
        readonly public static uint DEAFAULT_DISCOVERY_PORT = 6932;


        public static bool CreateListenServer(uint port)
        {
            return InternalCalls.Networking_CreateListenServer(port);
        }

        public static bool CreateClient(bool LAN)
        {
            return InternalCalls.Networking_CreateClient(LAN);
        }

        public static bool ConnectTo(string server, uint port)
        {
            return InternalCalls.Networking_ConnectTo(server, port);
        }

        public static bool ConnectToLAN(string server)
        {
            return InternalCalls.Networking_ConnectToLAN(server);
        }

        public static uint InstanceID()
        {
            return InternalCalls.Networking_InstanceID();
        }

        static void OnClientConnect(uint handle)
        {
            Debug.Log("New Client");
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

        public static void InvokeRPC(Trace.Action src, UInt64 func_name_id, RPCType type, params object[] args)// TODO: Allow for Parameters
        {
            InternalCalls.Networking_InvokeRPC( src.GetID(), src, func_name_id, type, args);
        }
        
        public static void InvokeRPC(Trace.Action src, string func_name, RPCType type, params object[] args)
        {
            InvokeRPC(src, Utils.HashString(func_name), type, args);
        }

        public static void SendScenePacket(float deltaTime)
        {
            InternalCalls.Networking_SendScenePacket(deltaTime);
        }

        public static float GetServerAverageRTT()
        {
            return InternalCalls.Networking_GetServerAverageRTT();
        }
        
        public static float GetClientAverageRTT(uint client_id)
        {
            return InternalCalls.Networking_GetClientAverageRTT(client_id);
        }
    }
}
