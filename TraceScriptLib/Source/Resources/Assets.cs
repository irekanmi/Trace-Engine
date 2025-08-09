using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Trace
{
    public interface Asset<T>
    {
        
        
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Prefab : Asset<Prefab>
    {
        private ulong Id;

        Prefab(ulong Id)
        {
            this.Id = Id;
        }

        public ulong GetID()
        {
            return this.Id;
        }
        

        
    }

}
