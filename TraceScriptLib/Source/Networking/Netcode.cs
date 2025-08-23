using System;
using System.Collections.Generic;


namespace Trace
{
    internal enum NetPacketInfo
    {
        Unknown,
        CurrentServerTick,
    }


    class NetworkManager : Trace.Action
    {
        private Prefab player_prefab;
        private Prefab network_controller_prefab;
        private bool has_net_instance = false;
        private int tick_rate = 30;

        private float accumulator = 0.0f;
        private uint current_tick = 0;
        private float fixed_dt = 0.0f;
        private NetworkPacketManager packet_manager;

        //Client Data
        private NetworkController client_controller;

        //Server Data
        private Dictionary<uint, NetworkController> controllers;

        public override void OnStart()
        {
            controllers = new Dictionary<uint, NetworkController>();

            if (Network.IsClient())
            {
                Network.ConnectTo("127.0.0.1", Network.DEAFAULT_SERVER_PORT);
                Network.on_server_connect += OnServerConnected;
            }

            if(Network.IsServer())
            {
                Scene.InstanciatePrefab_Net(player_prefab, new Vec3(0.0f, 3.0f, 0.0f), Network.InstanceID());
                Scene.InstanciatePrefab_Net(network_controller_prefab, new Vec3(0.0f, 0.0f, 0.0f), Network.InstanceID());
                Network.on_client_connect += OnClientConnected;
            }

            has_net_instance = Network.IsClient() || Network.IsServer();

            if(has_net_instance)
            {
                Scene.StimulatePhysics = false;
            }
            else
            {
                Scene.StimulatePhysics = true;
            }

            current_tick = 0;

        }

        public override void OnCreate()
        {
            fixed_dt = 1.0f / (float)tick_rate;
        }

        public override void OnUpdate(float deltaTime)
        {
            //TEMP -------------------------------------
            if (!has_net_instance)
            {

                if (Input.GetKeyReleased(Keys.KEY_H))
                {
                    Network.CreateListenServer(Network.DEAFAULT_SERVER_PORT);
                    Application.LoadAndSetScene("GameScene.trscn");
                    has_net_instance = true;
                }

                if (Input.GetKeyReleased(Keys.KEY_J))
                {
                    Network.CreateClient(false);
                    Application.LoadAndSetScene("GameScene.trscn");
                    has_net_instance = true;
                }

            }

            if(Network.IsServer())
            {
                HandleServerUpdate(deltaTime);
            }
            
            if(Network.IsClient())
            {
                HandleClientUpdate(deltaTime);
            }

        }

        private void HandleServerUpdate(float deltaTime)
        {
            accumulator += deltaTime;
            while(accumulator >= fixed_dt)
            {
                //Because it is a listen server
                if (client_controller != null)
                {
                    client_controller.SampleInput(current_tick);
                }

                Scene.IterateComponent<NetObject>(this, "UpdateNetObjects");

                Physics.Step(fixed_dt);

                current_tick += 1;
                accumulator -= fixed_dt;
            }
        }

        private void HandleClientUpdate(float deltaTime)
        {
            accumulator += deltaTime;
            while (accumulator >= fixed_dt)
            {
                if (client_controller.IsValid())
                {
                    client_controller.SampleInput(current_tick);
                }

                Scene.IterateComponent<NetObject>(this, "UpdateNetObjects");

                Physics.Step(fixed_dt);

                current_tick++;
                accumulator -= fixed_dt;
            }
        }

        public int GetTickRate()
        {
            return tick_rate;
        }

        public uint GetCurrentTick()
        {
            return current_tick;
        }

        private void OnServerConnected(uint net_id)
        {

        }
        
        private void OnClientConnected(uint net_id)
        {
            Scene.InstanciatePrefab_Net(player_prefab, new Vec3(0.0f, 3.0f, 0.0f), net_id);
            Scene.InstanciatePrefab_Net(network_controller_prefab, new Vec3(0.0f, 0.0f, 0.0f), net_id);
        }

        private void UpdateNetObjects(Trace.Action obj, NetObject net_instance)
        {
            if (Network.IsServer())
            {
                Scene.IterateEntityScripts(obj, this, "StimulateObjects_Server");
            }
            
            if (Network.IsClient())
            {
                Scene.IterateEntityScripts(obj, this, "StimulateObjects_Client");
            }
        }

        private void StimulateObjects_Server(Trace.Action obj)
        {
            if(controllers.TryGetValue(obj.GetNetID(), out NetworkController obj_controller))
            {
                obj.Stimulate(fixed_dt, current_tick, obj_controller);
            }
            else
            {
                obj.Stimulate(fixed_dt, current_tick);
            }
        }
        
        private void StimulateObjects_Client(Trace.Action obj)
        {
            if (!obj.IsOwner())
            {
                return;
            }
            obj.Stimulate(fixed_dt, current_tick, client_controller.IsValid() ? client_controller : null);


        }

        public void SetPacketManager(NetworkPacketManager manager)
        {
            packet_manager = manager;
        }

        public void SetNetworkController(Trace.Action instance_controller)
        {
            if(Network.IsClient()  && instance_controller.IsOwner())
            {
                client_controller = (NetworkController)instance_controller;
            }

            if(Network.IsServer())
            {
                //because this is a listen server
                if (instance_controller.IsOwner())
                {
                    client_controller = (NetworkController)instance_controller;
                }

                controllers.Add( instance_controller.GetNetID(), (NetworkController)instance_controller);
            }
        }

        public void SetServerTick(uint server_tick)
        {
            if(!Network.IsClient())
            {
                return;
            }

            //Determine this number of ticks the client should be ahead of server
            float time_offset = 0.1f;// 100ms
            uint offset_tick = (uint)(time_offset / fixed_dt);
            current_tick = server_tick + offset_tick;
            Debug.Trace("Client Manager Current Tick:" + current_tick.ToString());
        }
    }

    class NetworkPacketManager : Trace.Action
    {
        private bool server_tick_received = false;
        public override void OnServerSend(ulong stream_handle)
        {
           if(server_tick_received)
            {
                return;
            }

           // It means that this is a listen server
           if(IsOwner())
            {
                server_tick_received = true;
                return;
            }

            NetPacketInfo header = NetPacketInfo.CurrentServerTick;
            NetworkManager manager = (NetworkManager)Scene.GetEntityWithScript<NetworkManager>();
            Stream.WriteInt(stream_handle, (int)header);
            Stream.WriteInt(stream_handle, (int)manager.GetCurrentTick());

        }

        public override void OnClientReceive(ulong stream_handle)
        {
            NetPacketInfo header = (NetPacketInfo)Stream.ReadInt(stream_handle);

            switch(header)
            {
                case NetPacketInfo.CurrentServerTick:
                    {
                        uint current_server_tick = (uint)Stream.ReadInt(stream_handle);
                        if(!IsOwner() || server_tick_received)
                        {
                            return;
                        }
                        NetworkManager manager = (NetworkManager)Scene.GetEntityWithScript<NetworkManager>();
                        manager.SetServerTick(current_server_tick);
                        Network.InvokeRPC(this, "ClientRecievedTick", RPCType.SERVER);
                        server_tick_received = true;
                        break;
                    }
            }
        }

        void ClientRecievedTick()
        {
            server_tick_received = true;
        }

    }
    class TransformSync : Trace.Action
    {

        public override void OnServerSend(UInt64 stream_handle)
        {
            TransformComponent pose = GetComponent<TransformComponent>();
            Stream.WriteVec3(stream_handle, pose.Position);
            Stream.WriteQuat(stream_handle, pose.Rotation);

        }        

        public override void OnClientReceive(UInt64 stream_handle)
        {

            Vec3 new_pos = Stream.ReadVec3(stream_handle);
            Quat new_rot = Stream.ReadQuat(stream_handle);


            if (IsOwner())
            {
                return;
            }
            else
            {
                TransformComponent pose = GetComponent<TransformComponent>();
                pose.Position = new_pos;
                pose.Rotation = new_rot;
            }

        }


    }


    public class NetworkController : Trace.Action
    {


        public override void OnNetworkCreate()
        {
            Trace.Action network_manager = Scene.GetEntityWithScript<NetworkManager>();
            if(!network_manager.IsValid())
            {
                return;
            }
            NetworkManager script = (NetworkManager)network_manager;

            script.SetNetworkController(this);
            NetworkPacketManager packet_manager = AddScript<NetworkPacketManager>();
            if (IsOwner())
            {
                script.SetPacketManager(packet_manager);
            }


        }


        public virtual void SampleInput(uint current_tick)
        {

        }

        public virtual bool HasInput(uint tick)
        {
            return true;
        }

    }

    public struct BasicInput
    {
        public Vec3 direction;
    }

    public class BasicController : NetworkController
    {
        List<BasicInput> inputs;
        uint buffer_size = 64;
        uint _tick = 0;

        public override void OnNetworkCreate()
        {
            base.OnNetworkCreate();
            inputs = new List<BasicInput>((int)buffer_size);
            for(int i = 0; i < (int)buffer_size; i++)
            {
                inputs.Add(new BasicInput { direction = Vec3.Zero });
            }


        }

        public override bool HasInput(uint tick)
        {
            bool result = (tick <= _tick);

            return result;
        }

        public override void SampleInput(uint current_tick)
        {
            Vec3 input = Vec3.Zero;
            if (Input.GetKey(Keys.KEY_W))
            {
                input.z += 1.0f;
            }

            if (Input.GetKey(Keys.KEY_S))
            {
                input.z -= 1.0f;
            }

            if (Input.GetKey(Keys.KEY_A))
            {
                input.x += 1.0f;
            }

            if (Input.GetKey(Keys.KEY_D))
            {
                input.x -= 1.0f;
            }

            int index = (int)(current_tick % buffer_size);
            inputs[index] = new BasicInput { direction = input };

            _tick = current_tick;
        }

        public BasicInput GetInput(uint tick)
        {
            int index = (int)(_tick % buffer_size);
            return inputs[index];
        }

        public override void OnClientSend(ulong stream_handle)
        {
            base.OnClientSend(stream_handle);

            if(Network.IsServer())
            {
                return;
            }

            Debug.Trace("Send Time Stamp " + _tick.ToString());

            Stream.WriteInt(stream_handle, (int)_tick);
            Stream.WriteInt(stream_handle, (int)buffer_size);

            for(int i = 0; i < buffer_size; i++)
            {
                Stream.WriteVec3(stream_handle, inputs[i].direction);
            }
        }

        public override void OnServerReceive(ulong stream_handle)
        {
            base.OnServerReceive(stream_handle);

            Debug.Trace("Recieve Time Stamp " + _tick.ToString());

            _tick = (uint)Stream.ReadInt(stream_handle);
            buffer_size = (uint)Stream.ReadInt(stream_handle);

            for (int i = 0; i < buffer_size; i++)
            {
                Vec3 dir = Stream.ReadVec3(stream_handle);
                inputs[i] = new BasicInput { direction = dir };
            }

        }

    }

}
