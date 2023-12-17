using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Trace;
class Player : Trace.Action
{
    public float Speed = 5.0f;
    

    public void OnCreate()
    {
        Console.WriteLine("Player Created");
        Speed -= 4.5f;
    }
    
    public void OnUpdate(float deltaTime)
    {
        Console.WriteLine($"Player Update Float {deltaTime}");
        Speed -= 1.1f;
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

