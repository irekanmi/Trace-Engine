using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Scene
    {
        static public Action GetEntityByName(ulong string_id)
        {
            object obj = InternalCalls.Scene_GetEntityByName(string_id);
            return obj as Action;
        }

        static public Action GetEntityByName(string str)
        {
            return GetEntityByName(Utils.HashString(str));
        }

        static public Action GetEntity(ulong entity_id)
        {
            object obj = InternalCalls.Scene_GetEntity(entity_id);
            return obj as Action;
        }

        static public Action InstanciateEntity(Trace.Action entity, Vec3 position)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Position(entity.GetID(), ref position);
            return obj as Action;
        }

        static public Action InstanciateEntity(ulong entity_id, Vec3 position)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Position(entity_id, ref position);
            return obj as Action;
        }



    }
}
