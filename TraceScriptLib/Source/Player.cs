using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Trace;
class Player : Trace.Action
{
    public float Speed = 5.5f;
    public ulong test_id = 8327832;

    public void OnStart()
    {
        Debug.Trace($"Player Started Id:{Id}");
    }

    public void OnCreate()
    {
        Debug.Info("Player Created");
    }
    
    public void OnUpdate(float deltaTime)
    {
        Trace.Action sad_idle = Scene.GetEntityByName("Sad Idle");
        TransformComponent pose = sad_idle.GetComponent<TransformComponent>();

        Vec3 dir = Vec3.Zero;
        if (Input.GetKey(Keys.KEY_W))
        {
            dir.y = 1.0f;
        }
        if (Input.GetKey(Keys.KEY_A))
        {
            dir.x = -1.0f;
        }
        if (Input.GetKey(Keys.KEY_S))
        {
            dir.y = -1.0f;
        }
        if (Input.GetKey(Keys.KEY_D))
        {
            dir.x = 1.0f;
        }
        dir *= Speed * deltaTime;

        Vec3 pos = pose.Position;
        pos += dir;
        pose.Position = pos;

    }   

    
    ~Player()
    {
        Debug.Log($"Player Destroyed, ID: {Id}");
    }


}

class GUID : Trace.Action
{
    public void OnEnable()
    {
        Debug.Trace($"Player Enabled Id:{Id}");
    }

    public void OnUpdate(float deltaTime)
    {
        //TextComponent txt = GetComponent<TextComponent>();
        //txt.Text = "Happy New Year \nFrom \nTrace Game Engine";
    }

}

class FollowEntity : Trace.Action
{
    private Trace.Action target;
    private Vec3 last_position = Vec3.Zero;
    public float follow_threshold = 1.0f;
    public void OnStart()
    {
        target = Scene.GetEntityByName("mixamorig:Head");
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        last_position = target_transform.WorldPosition;
    }

    public void OnUpdate(float deltaTime)
    {
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        Vec3 current_position = target_transform.WorldPosition;
        if (last_position != current_position)
        {
            Vec3 diff = current_position - last_position;
            diff *= follow_threshold;
            last_position = current_position;
            TransformComponent transform = GetComponent<TransformComponent>();
            Vec3 position = transform.Position;
            position += diff;
            transform.Position = position;
        }

        if(Input.GetKeyPressed(Keys.KEY_F))
        {
            Scene.InstanciateEntity(Scene.GetEntityByName("Sad Idle"), new Vec3(0.0f, 0.0f, 0.0f));
        }
    }

}


class FollowXBot : Trace.Action
{
    private Trace.Action target;
    private Vec3 last_position = Vec3.Zero;
    public float follow_threshold = 1.0f;
    public void OnEnable()
    {
        Trace.Action x_bot = Scene.GetEntityByName("X_bot_1");
        target = x_bot.GetChildWithName("mixamorig:Head");
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        last_position = target_transform.WorldPosition;
    }

    public void OnUpdate(float deltaTime)
    {
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        Vec3 current_position = target_transform.WorldPosition;
        if (last_position != current_position)
        {
            Vec3 diff = current_position - last_position;
            diff *= follow_threshold;
            last_position = current_position;
            TransformComponent transform = GetComponent<TransformComponent>();
            Vec3 position = transform.Position;
            position += diff;
            transform.Position = position;
        }
    }

}

class FollowYBot : Trace.Action
{
    private Trace.Action target;
    private Vec3 last_position = Vec3.Zero;
    public float follow_threshold = 1.0f;
    public void OnEnable()
    {
        Trace.Action x_bot = Scene.GetEntityByName("Y_Bot_1");
        target = x_bot.GetChildWithName("mixamorig:Hips");
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        last_position = target_transform.WorldPosition;
    }

    public void OnUpdate(float deltaTime)
    {
        TransformComponent target_transform = target.GetComponent<TransformComponent>();
        Vec3 current_position = target_transform.WorldPosition;
        if (last_position != current_position)
        {
            Vec3 diff = current_position - last_position;
            diff *= follow_threshold;
            last_position = current_position;
            TransformComponent transform = GetComponent<TransformComponent>();
            Vec3 position = transform.Position;
            position += diff;
            transform.Position = position;
        }
    }

}


class NetController : Trace.Action
{
    uint client_handle = 0;
    public void OnStart()
    {
        if(Network.IsClient())
        {
            Network.ConnectTo("127.0.0.1", Network.DEAFAULT_SERVER_PORT);
        }

        if (!Network.IsServer())
        {
            return;
        }

        Network.on_client_connect += OnClientConnect;
        Debug.Log("Client Connect Action Added");
    }

    void OnClientConnect(uint connection_handle)
    {
        
        if(!Network.IsServer())
        {
            return;
        }
        client_handle = connection_handle;
        //Scene.InstanciateEntity_Net(Scene.GetEntityByName("X_bot"), new Vec3(1.0f, 0.0f, 1.0f), connection_handle);
    }

    public void OnUpdate(float deltaTime)
    {
        if (!Network.IsServer() && !Network.IsClient())
        {

            if (Input.GetKeyReleased(Keys.KEY_H))
            {
                Network.CreateListenServer(Network.DEAFAULT_SERVER_PORT);
                Application.LoadAndSetScene("Networked_Balls.trscn");
            }

            if (Input.GetKeyReleased(Keys.KEY_J))
            {
                Network.CreateClient(false);
                Application.LoadAndSetScene("Networked_Balls.trscn");
            }
        }


        if (Network.IsClient() && Input.GetKeyPressed(Keys.KEY_F))
        {
            Network.InvokeRPC( this ,"TestServerRPC", RPCType.SERVER);
        }

        if (Network.IsServer() && Input.GetKeyPressed(Keys.KEY_F))
        {
            //Network.InvokeRPC(this, "TestClientRPC", RPCType.CLIENT);
            Scene.InstanciateEntity_Net(Scene.GetEntityByName("X_bot"), new Vec3(1.0f, 0.0f, 1.0f), client_handle);
        }


    }

    void TestClientRPC()
    {
        Debug.Log("A Client RPC from a server");
    }
    
    void TestServerRPC()
    {
        Debug.Log("A Server RPC from a client");
        //Scene.InstanciateEntity(Scene.GetEntityByName("Sphere_Ball"), new Vec3(1.0f, 22.0f, 0.0f));
    }

}

class TransformSync : Trace.Action
{
    bool override_data = false;

    public override void OnServerSend(UInt64 stream_handle)
    {
        TransformComponent pose = GetComponent<TransformComponent>();
        Stream.WriteVec3(stream_handle, pose.Position);
        Stream.WriteQuat(stream_handle, pose.Rotation);

    }
    
    public override void OnClientSend(UInt64 stream_handle)
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
    
    public override void OnServerReceive(UInt64 stream_handle)
    {

        Vec3 new_pos = Stream.ReadVec3(stream_handle);
        Quat new_rot = Stream.ReadQuat(stream_handle);
        
        
        if (!IsOwner())
        {
            TransformComponent pose = GetComponent<TransformComponent>();
            pose.Position = new_pos;
            pose.Rotation = new_rot;
            return;
        }


    }


}


