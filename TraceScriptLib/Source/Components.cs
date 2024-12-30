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

}
