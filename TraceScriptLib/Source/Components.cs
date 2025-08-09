using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public interface Component<T> //: EntityObject
    {
        T Create(ulong id);
        
    }

    public struct TransformComponent : Component<TransformComponent>
    {
        private ulong Id;

        TransformComponent(ulong Id)
        {
            this.Id = Id;
        }
        public Vec3 Position
        {
            get
            {
                InternalCalls.TransformComponent_GetPosition(Id, out Vec3 position);
                return position;
            }
            set
            {
                InternalCalls.TransformComponent_SetPosition(Id,ref value);
            }
        }

        public Vec3 WorldPosition
        {
            get
            {
                InternalCalls.TransformComponent_GetWorldPosition(Id, out Vec3 position);
                return position;
            }
            set
            {
                InternalCalls.TransformComponent_SetWorldPosition(Id, ref value);
            }
        }

        public Quat Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Id, out Quat rotation);
                return rotation;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(Id, ref value);
            }
        }

        public Vec3 Scale
        {
            get
            {
                return Vec3.Zero;
            }
            set
            {

            }
        }

        public Vec3 Forward 
        {
            get
            {
                InternalCalls.TransformComponent_Forward(Id, out Vec3 result);
                return result;
            }
        }
        
        public Vec3 Right 
        {
            get
            {
                InternalCalls.TransformComponent_Right(Id, out Vec3 result);
                return result;
            }
        }
        
        public Vec3 Up 
        {
            get
            {
                InternalCalls.TransformComponent_Up(Id, out Vec3 result);
                return result;
            }
        }

        public TransformComponent Create(ulong id)
        {
            TransformComponent component = new TransformComponent(id);
            return component;
        }

    }

    public struct TextComponent : Component<TextComponent>
    {
        private ulong Id;

        TextComponent(ulong Id)
        {
            this.Id = Id;
        }
        public string Text
        {
            get
            {
                return InternalCalls.TextComponent_GetString(Id);
            }
            set
            {
                InternalCalls.TextComponent_SetString(Id, value);
            }
        }

        public TextComponent Create(ulong id)
        {
            return new TextComponent(id);
        }
    }

    public struct CharacterControllerComponent : Component<CharacterControllerComponent>
    {
        private ulong Id;

        CharacterControllerComponent(ulong Id)
        {
            this.Id = Id;
        }

        public CharacterControllerComponent Create(ulong id)
        {
            CharacterControllerComponent component = new CharacterControllerComponent(id);
            return component;
        }

        public bool IsGrounded
        {
            get
            {
                return InternalCalls.CharacterController_IsGrounded(Id);
            }
        }

        public void Move(Vec3 displacement, float deltaTime)
        {
            InternalCalls.CharacterController_Move(Id, ref displacement, deltaTime);
        }


    }

    public struct AnimationGraphController : Component<AnimationGraphController>
    {
        private ulong Id;

        AnimationGraphController(ulong Id)
        {
            this.Id = Id;
        }

        public AnimationGraphController Create(ulong id)
        {
            AnimationGraphController component = new AnimationGraphController(id);
            return component;
        }

        public void SetParameterBool(string name, bool value)
        {
            InternalCalls.AnimationGraphController_SetParameterBool(Id, name, value);
        }

    }




}


