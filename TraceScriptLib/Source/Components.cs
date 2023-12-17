using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Component
    {
        public Action entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vec3 Position
        {
            get
            {
                return Vec3.Zero;
            }
            set
            {

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
}
