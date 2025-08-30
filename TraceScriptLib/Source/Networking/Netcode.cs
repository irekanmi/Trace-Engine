using System;
using System.Collections.Generic;


namespace Trace
{
    internal enum NetPacketInfo
    {
        Unknown,
        CurrentServerTick,
    }


    public class NetworkManager : Trace.Action
    {
        private Prefab player_prefab;
        private Prefab network_controller_prefab;
        private bool has_net_instance = false;
        private int tick_rate = 30;
        private float interpolation_offset = 0.1f;
        private int server_packet_rate = 30;
        private int client_packet_rate = 30;

        private float accumulator = 0.0f;
        private uint current_tick = 0;
        private uint received_server_tick = 0;
        private float server_time = 0.0f;
        private float fixed_dt = 0.0f;
        private NetworkPacketManager packet_manager;
        private bool reconcile_client = false;
        private uint reconcile_tick = 0;
        private float elasped_send_time = 0.0f;


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
                Scene.InstanciatePrefab_Net(player_prefab, new Vec3(0.0f, 0.0f, 0.0f), Network.InstanceID());
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

        public override void OnDestroy()
        {
            if (Network.IsClient())
            {
                Network.on_server_connect -= OnServerConnected;
            }

            if (Network.IsServer())
            {
                Network.on_client_connect -= OnClientConnected;
            }
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

                float send_duration = 1.0f / (float)server_packet_rate;
                if(elasped_send_time >= send_duration)
                {
                    //trigger packet send
                    Network.SendScenePacket(deltaTime);

                    elasped_send_time = 0.0f;
                }

            }
            
            if(Network.IsClient())
            {
                HandleClientUpdate(deltaTime);

                server_time += deltaTime;
                float send_duration = 1.0f / (float)client_packet_rate;
                if (elasped_send_time >= send_duration)
                {
                    //trigger packet send
                    Network.SendScenePacket(deltaTime);

                    elasped_send_time = 0.0f;
                }
            }

            elasped_send_time += deltaTime;

        }

        private void HandleServerUpdate(float deltaTime)
        {
            accumulator += deltaTime;
            while(accumulator >= fixed_dt)
            {
                //Because it is a listen server
                if (client_controller.IsValid())
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

        public float GetFixedDelta()
        {
            return fixed_dt;
        }

        public uint GetCurrentTick()
        {
            return current_tick;
        }

        public float GetInterpolationOffset()
        {
            return interpolation_offset;
        }

        public float GetServerTime()
        {
            return server_time;
        }

        private void OnServerConnected(uint net_id)
        {

        }
        
        private void OnClientConnected(uint net_id)
        {
            Scene.InstanciatePrefab_Net(player_prefab, new Vec3(0.0f, 0.0f, 0.0f), net_id);
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
            bool is_predicted_action = obj is ReplicatedAction;

            if(!is_predicted_action)
            {
                return;
            }

            ReplicatedAction entity = (ReplicatedAction)obj;

            if(controllers.TryGetValue(obj.GetNetID(), out NetworkController obj_controller))
            {
                if(!entity.IsOwner())
                {
                }
                entity.Step(fixed_dt, current_tick, obj_controller);
            }
            else
            {
                entity.Step(fixed_dt, current_tick, null);
            }
        }
        
        private void StimulateObjects_Client(Trace.Action obj)
        {
            if (!obj.IsOwner())
            {
                return;
            }

            bool is_predicted_action = obj is ReplicatedAction;

            if (!is_predicted_action)
            {
                return;
            }

            ReplicatedAction entity = (ReplicatedAction)obj;

            entity.Step(fixed_dt, GetStepTick(), client_controller.IsValid() ? client_controller : null);


        }

        private void ReconcileClientEntities()
        {
            ++reconcile_tick;
            while (reconcile_tick < current_tick)
            {
                Scene.IterateComponent<NetObject>(this, "UpdateNetObjects");
                Physics.Step(fixed_dt);

                reconcile_tick++;
            }
            Scene.IterateComponent<NetObject>(this, "IterateClientEntites_Reconcile");
        }

        private void IterateClientEntites(Trace.Action obj)
        {
            if(!obj.IsOwner())
            {
                return;
            }

            Scene.IterateEntityScripts(obj, this, "IterateClientEntityScripts");
        }
        
        private void IterateClientEntites_Reconcile(Trace.Action obj)
        {
            if(!obj.IsOwner())
            {
                return;
            }

            Scene.IterateEntityScripts(obj, this, "IterateClientEntityScripts_Reconcile");
        }
        private void IterateClientEntityScripts_Reconcile(Trace.Action obj)
        {
            if (!obj.IsOwner())
            {
                return;
            }

            bool is_predicted_action = obj is ReplicatedAction;

            if (!is_predicted_action)
            {
                return;
            }

            ReplicatedAction entity = (ReplicatedAction)obj;

            entity.ApplyReconciliation();
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

            received_server_tick = server_tick;
            server_time = (float)server_tick * fixed_dt;// Account for latency

            //Determine this number of ticks the client should be ahead of server
            float time_offset = 0.2f;// 100ms
            uint offset_tick = (uint)(time_offset / fixed_dt);
            current_tick = server_tick + offset_tick;
            Debug.Trace("Client Manager Current Tick:" + current_tick.ToString());
        }

        private uint GetStepTick()
        {
            if(reconcile_client)
            {
                return reconcile_tick;
            }

            return current_tick;
        }

        public void TriggerReconciliation(uint tick)
        {
            reconcile_tick = tick;
            reconcile_client = true;

            ReconcileClientEntities();

            reconcile_client = false;
        }
    }

    public class NetworkPacketManager : Trace.Action
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
                        NetworkManager manager = (NetworkManager)Scene.GetEntityWithScript<NetworkManager>();
                        manager.SetServerTick(current_server_tick);
                        if(!IsOwner() || server_tick_received)
                        {
                            return;
                        }
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
        public uint tick_frame;
    }

    public class BasicController : NetworkController
    {
        List<BasicInput> inputs;
        Dictionary<uint, BasicInput> outputs;
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

            if(Network.IsServer())
            {
                outputs = new Dictionary<uint, BasicInput>();
            }

        }

        public override bool HasInput(uint tick)
        {
            bool result = (tick <= _tick);

            if(Network.IsServer() && !IsOwner())
            {
                result = outputs.ContainsKey(tick);
            }

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
            inputs[index] = new BasicInput { direction = input, tick_frame = current_tick };

            _tick = current_tick;
        }

        public BasicInput GetInput(uint tick)
        {

            int index = (int)(_tick % buffer_size);
            if(Network.IsClient())
            {
                return inputs[index];
            }

            if(Network.IsServer() && IsOwner())
            {
                return inputs[index];
            }

            if(Network.IsServer() && !IsOwner())
            {
                BasicInput result = outputs[tick];
                outputs.Remove(tick);
                return result;
            }


            return new BasicInput();
        }

        public override void OnClientSend(ulong stream_handle)
        {
            base.OnClientSend(stream_handle);

            if(Network.IsServer())
            {
                return;
            }


            Stream.WriteInt(stream_handle, (int)_tick);
            Stream.WriteInt(stream_handle, (int)buffer_size);

            for(int i = 0; i < buffer_size; i++)
            {
                Stream.WriteInt(stream_handle, (int)inputs[i].tick_frame);
                Stream.WriteVec3(stream_handle, inputs[i].direction);
            }
        }

        public override void OnServerReceive(ulong stream_handle)
        {
            base.OnServerReceive(stream_handle);

            NetworkManager manager = (NetworkManager)Scene.GetEntityWithScript<NetworkManager>();

            _tick = (uint)Stream.ReadInt(stream_handle);
            buffer_size = (uint)Stream.ReadInt(stream_handle);

            for (int i = 0; i < buffer_size; i++)
            {
                uint tick = (uint)Stream.ReadInt(stream_handle);
                Vec3 dir = Stream.ReadVec3(stream_handle);
                
                if(tick <= manager.GetCurrentTick())
                {
                    continue;
                }
                if (!outputs.ContainsKey(tick))
                {
                    outputs.Add(tick, new BasicInput { direction = dir, tick_frame = tick });
                }
            }

        }

    }

    public interface IActionData
    {
        void Serialize(ulong stream_handle, Trace.Action owner);
        void Deserialize(ulong stream_handle, Trace.Action owner);
    }

    public class ReplicatedAction : Trace.Action
    {
        public virtual void Step(float fixed_deltaTime, uint tick, NetworkController controller = null)
        {

        }
        
        public virtual void ApplyReconciliation()
        {

        }
    }
    public class PredictedAction<ActionDataType, ControllerType> : Trace.ReplicatedAction 
        where ActionDataType : struct, IActionData
        where ControllerType : Trace.NetworkController
    {
        protected ActionDataType action_state;
        protected ActionDataType prev_action_state;
        protected uint last_state_tick;
        protected ActionDataType[] client_state_history;
        protected int client_state_history_count = 64;
        protected RingBuffer<ActionDataType> remote_state_history;
        protected RingBuffer<uint> remote_state_ticks;
        protected int remote_state_count = 12;
        protected NetworkManager manager;

        public override void OnNetworkCreate()
        {
            action_state = new ActionDataType();
            prev_action_state = new ActionDataType();

            manager = (NetworkManager)Scene.GetEntityWithScript<NetworkManager>();

            if (IsOwner())
            {
                client_state_history_count = (int)(2.5f * (float)manager.GetTickRate());
                client_state_history = new ActionDataType[client_state_history_count];
            }
            else
            {
                remote_state_history = new RingBuffer<ActionDataType>(remote_state_count);
                remote_state_ticks = new RingBuffer<uint>(remote_state_count);
            }

        }

        public override void Step(float fixed_deltaTime, uint tick, NetworkController controller = null)
        {
            Stimulate(fixed_deltaTime, tick, (ControllerType)controller);
            last_state_tick = tick;
            if (IsOwner())
            {
                int index = (int)tick % client_state_history_count;
                client_state_history[index] = action_state;
            }
        }
        public override void ApplyReconciliation()
        {
            prev_action_state = action_state;
        }

        public virtual void Stimulate(float fixed_deltaTime, uint tick, ControllerType controller = null)
        {
        }

        protected void AddRemoteState(ref ActionDataType new_state, uint tick)
        {
            if(remote_state_history.Full)
            {
                remote_state_history.PopFront();
            }
            remote_state_history.PushBack(new_state);

            if(remote_state_ticks.Full)
            {
                remote_state_ticks.PopFront();
            }
            remote_state_ticks.PushBack(tick);
        }

        protected bool GetRemoteState(float time, out ActionDataType a, out ActionDataType b, out float t)
        {
            
            int index = 0;
            a = action_state;
            b = action_state;
            t = 0.0f;
            while(remote_state_ticks.Iterate(index, out uint value))
            {
                float b_time = (float)value * manager.GetFixedDelta();
                if(time <= b_time && index > 0)
                {
                    float a_time = (float)remote_state_ticks.Get(index - 1) * manager.GetFixedDelta();
                    a = remote_state_history.Get(index - 1);
                    b = remote_state_history.Get(index);

                    t = (time - a_time) / (b_time - a_time);

                    return true;
                }
                index++;
            }

            return false;
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

}
