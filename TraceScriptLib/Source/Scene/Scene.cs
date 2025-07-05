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
            return InstanciateEntity(entity.GetID(), position);
        }

        static public Action InstanciateEntity(ulong entity_id, Vec3 position)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Position(entity_id, ref position);
            return obj as Action;
        }
        static public Action InstanciateEntity_Net(Trace.Action entity, Vec3 position, uint owner_handle)
        {
            return InstanciateEntity_Net(entity.GetID(), position,owner_handle);
        }

        static public Action InstanciateEntity_Net(ulong entity_id, Vec3 position, uint owner_handle)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Position_NetID(entity_id, ref position, owner_handle);
            return obj as Action;
        }
        
        static public void DestroyEntity(ulong entity_id)
        {
            InternalCalls.Scene_DestroyEntity(entity_id);
        }
        static public void DestroyEntity(Trace.Action entity)
        {
            DestroyEntity(entity.GetID());
        }

        static public void EnableEntity(ulong entity_id)
        {
            InternalCalls.Scene_EnableEntity(entity_id);
        }
        static public void EnableEntity(Trace.Action entity)
        {
            EnableEntity(entity.GetID());
        }

        static public void DisableEntity(ulong entity_id)
        {
            InternalCalls.Scene_DisableEntity(entity_id);
        }
        static public void DisableEntity(Trace.Action entity)
        {
            DisableEntity(entity.GetID());
        }



    }
}
