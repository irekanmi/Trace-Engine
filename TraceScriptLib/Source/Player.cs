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
        TransformComponent pose = GetComponent<TransformComponent>();

        Vec3 dir = Vec3.Zero;
        if(Input.GetKey(Keys.KEY_W))
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

    public void OnCollisionEnter(Trace.Collision collision)
    {
        float z = collision.position.y;
        ulong id = collision.other.Id;
        Console.WriteLine($"Direction x:{collision.position.z},  {id}");
        Debug.Trace("Player Collision entered");
        Debug.Log("Player Collision entered");
        Debug.Info("Player Collision entered");
    }

}

