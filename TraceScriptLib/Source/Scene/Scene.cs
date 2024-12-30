using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Scene
    {
        static public Action GetEnityByName(ulong string_id)
        {
            object obj = InternalCalls.Scene_GetEntityByName(string_id);
            return obj as Action;
        }

        static public Action GetEnityByName(string str)
        {
            return GetEnityByName(Utils.HashString(str));
        }
    }
}
