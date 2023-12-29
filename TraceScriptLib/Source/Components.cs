using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Component
    {
        public ulong entityId { get; protected set; }

        protected Component()
        {
            entityId = 0;
        }

        internal protected Component(ulong ent)
        {
            entityId = ent;
        }
    }

    public class TransformComponent : Component
    {
        public Vec3 Position
        {
            get
            {
                InternalCalls.TransformComponent_GetPosition(entityId, out Vec3 position);
                return position;
            }
            set
            {
                InternalCalls.TransformComponent_SetPosition(entityId,ref value);
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

    }

    public class TextComponent : Component
    {

        public string Text { 
            get
            {
                return InternalCalls.TextComponent_GetString(entityId);
            }
            set
            {
                InternalCalls.TextComponent_SetString(entityId, value);
            }
        }
    }

}
