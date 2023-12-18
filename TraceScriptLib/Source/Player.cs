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
        Speed -= 4.5f;
    }

    public void OnCreate()
    {
        Debug.Info("Player Created");
        Speed -= 4.5f;
    }
    
    public void OnUpdate(float deltaTime)
    {
        //Console.WriteLine($"Player Update Float {deltaTime}");
        Speed -= deltaTime;
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

