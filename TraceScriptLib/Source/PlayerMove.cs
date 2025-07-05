using System;
using System.Collections.Generic;
using Trace;

public class PlayerMove : Trace.Action
{

    private Vec2 move_dir;
    private AnimationGraphController anim_controller;
    private TransformComponent pose;
    private string walk_id = "IsWalking";
    private string run_id = "IsRunning";
    private bool is_run_pressed = false;
    private bool is_jump_pressed = false;
    private bool is_moving = false;


    public float speed = 3.0f;
    public float speed_factor = 3.0f;
    public float rotation_factor_per_frame = 15.0f;
    Vec3 move_diplacement;

    void OnStart()
    {
        anim_controller = GetComponent<AnimationGraphController>();
        pose = GetComponent<TransformComponent>();
        move_diplacement = Vec3.Zero;
    }

    public override void OnNetworkCreate()
    {
        anim_controller = GetComponent<AnimationGraphController>();
        pose = GetComponent<TransformComponent>();
        move_diplacement = Vec3.Zero;
    }

    void OnUpdate(float deltaTime)
    {
        if(!IsOwner())
        {
            return;
        }

        HandleMovement();
        HandleRotation(deltaTime);
        HandleAnimation();


        move_diplacement.x *= speed;
        move_diplacement.z *= speed;


        Vec3 pos = pose.Position;
        pos += move_diplacement * deltaTime;

        pose.Position = pos;
    }

    private void HandleMovement()
    {

        if (Input.GetKeyPressed(Keys.KEY_W))
        {
            move_dir.y = 1.0f;
            is_moving = true;
        }

        if (Input.GetKeyPressed(Keys.KEY_S))
        {
            move_dir.y = -1.0f;
            is_moving = true;
        }

        if (Input.GetKeyPressed(Keys.KEY_A))
        {
            move_dir.x = -1.0f;
            is_moving = true;
        }

        if (Input.GetKeyPressed(Keys.KEY_D))
        {
            move_dir.x = 1.0f;
            is_moving = true;
        }

        if (Input.GetKeyReleased(Keys.KEY_W))
        {
            move_dir.y = 0.0f;
        }

        if (Input.GetKeyReleased(Keys.KEY_S))
        {
            move_dir.y = 0.0f;
        }

        if (Input.GetKeyReleased(Keys.KEY_A))
        {
            move_dir.x = 0.0f;
        }

        if (Input.GetKeyReleased(Keys.KEY_D))
        {
            move_dir.x = 0.0f;

        }

        if (Input.GetKeyPressed(Keys.KEY_LSHIFT))
        {
            is_run_pressed = true;
            speed *= speed_factor;
        }

        if (Input.GetKeyReleased(Keys.KEY_LSHIFT))
        {
            is_run_pressed = false;
            speed /= speed_factor;
        }

        if (move_dir == Vec2.Zero)
        {
            is_moving = false;
        }

        move_diplacement.x = move_dir.x;
        move_diplacement.z = move_dir.y;



    }

    private void HandleAnimation()
    {
        if (is_moving)
        {
            anim_controller.SetParameterBool(walk_id, is_moving);
            anim_controller.SetParameterBool(run_id, is_run_pressed);
        }
        else
        {
            anim_controller.SetParameterBool(walk_id, false);
            anim_controller.SetParameterBool(run_id, false);
        }
    }


    private void HandleRotation(float deltaTime)
    {
        Vec3 position_to_look_at = Vec3.Zero;

        position_to_look_at.x = move_dir.x;
        position_to_look_at.y = 0.0f;
        position_to_look_at.z = move_dir.y;

        Quat curr_rotation = pose.Rotation;

        if (is_moving)
        {
            Quat target_rotation = Quat.LookDirection(position_to_look_at);
            Quat new_rotation = Quat.Slerp(curr_rotation, target_rotation, rotation_factor_per_frame * deltaTime);
            pose.Rotation = new_rotation;
        }


    }




}