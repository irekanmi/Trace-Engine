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

    public void OnTriggerEnter(Trace.TriggerPair collision)
    {
        if (collision.triggerEntity.HasComponent<TextComponent>())
        {
            TextComponent txt = collision.triggerEntity.GetComponent<TextComponent>();
            txt.Text = "Has Trigger";
        }
    }

    public void OnTriggerExit(Trace.TriggerPair collision)
    {
        collision.otherEntity.GetScript<Player>().Speed = 10.5f;

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