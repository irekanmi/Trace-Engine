using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Scene
    {
        static public bool StimulatePhysics
        {
            get
            {
                return InternalCalls.Scene_GetStimulatePhysics();
            }
            set
            {
                InternalCalls.Scene_SetStimulatePhysics(value);
            }
        }
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
        
        static public Action GetEntityWithComponent<T>()
        {
            Type type = typeof(T);
            object obj = InternalCalls.Scene_GetEntityWithComponent(type);
            return obj as Action;
        }
        
        static public Action GetEntityWithScript<T>()
        {
            Type type = typeof(T);
            object obj = InternalCalls.Scene_GetEntityWithScript(type);
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
        static public Action InstanciatePrefab(Prefab prefab, Vec3 position)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Prefab_Position( prefab.GetID() , ref position);
            return obj as Action;
        }
        
        static public Action InstanciatePrefab_Net(Prefab prefab, Vec3 position, uint owner_id)
        {
            object obj = InternalCalls.Scene_InstanciateEntity_Prefab_Position_NetID( prefab.GetID() , ref position, owner_id);
            return obj as Action;
        }
        
        static public Action InstanciatePrefab(Prefab prefab, Vec3 position, Quat orientation)
        {
            Action result = InstanciatePrefab(prefab, position);
            if(result != null)
            {
                TransformComponent transform = result.GetComponent<TransformComponent>();
                transform.WorldRotation = orientation;
            }
            return result;
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


        static public void IterateComponent<T>(Trace.Action src, string func_name)
        {
            Type comp_type = typeof(T);
            InternalCalls.Scene_IterateComponent(src.GetID(), src, Utils.HashString(func_name), comp_type);
        }
        static public void IterateScript<T>(Trace.Action src, string func_name)
        {
            Type comp_type = typeof(T);
            InternalCalls.Scene_IterateScript(src.GetID(), src, Utils.HashString(func_name), comp_type);
        }

        static public void IterateEntityScripts(Trace.Action entity, Trace.Action src, string func_name)
        {
            InternalCalls.Scene_IterateEntityScripts(entity.GetID(), src.GetID(), src, Utils.HashString(func_name));
        }


    }
}
