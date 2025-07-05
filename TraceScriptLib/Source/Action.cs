using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Action 
    {
        protected ulong Id;
        protected Action() { Id = 0; }

        internal protected Action(ulong id)
        {
            Id = id;
        }

        public ulong GetID()
        {
            return Id;
        }

        public string GetName()
        {
            return InternalCalls.Action_GetName(Id);
        }

        public T GetComponent<T>() where T : Component<T>, new()
        {
            if (!HasComponent<T>()) return default(T);
            return new T().Create(Id);
        }

        public T GetScript<T>() where T : Action
        {
            if (!HasScript<T>()) return null;
            Type script_type = typeof(T);
            object script = InternalCalls.Action_GetScript(Id, script_type);
            return script as T;
        }

        public bool HasComponent<T>() where T : Component<T>
        {
            Type component_type = typeof(T);
            return InternalCalls.Action_HasComponent(Id, component_type);
        }

        public bool HasScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            return InternalCalls.Action_HasScript(Id, script_type);
        }

        public T AddComponent<T>() //where T : Component, new()
        {
            Type component_type = typeof(T);
            return default(T); // TODO: Implement AddComponent
        }

        public T AddScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            object obj = InternalCalls.Action_AddScript(Id, script_type);
            return obj as T;
        }

        public void RemoveComponent<T>() //where T : Component, new()
        {
            Type component_type = typeof(T);
            InternalCalls.Action_RemoveComponent(Id, component_type);
            return;
        }

        public void RemoveScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            InternalCalls.Action_RemoveScript(Id, script_type);
            return;
        }

        public Action GetChildWithName(ulong str_id)
        {
            object obj = InternalCalls.Scene_GetChildEntityByName(Id, str_id);
            return obj as Action;
        }

        public Action GetChildWithName(string name)
        {
            return GetChildWithName(Utils.HashString(name));
        }

        public virtual void OnCollisionEnter(CollisionData collision_data)
        {}

        public virtual void OnCollisionExit(CollisionData collision_data)
        {}
        
        public virtual void OnTriggerEnter(TriggerPair pair)
        {}
        
        public virtual void OnTriggerExit(TriggerPair pair)
        {

        }

        //Networking ----------------------------------
        public virtual void OnNetworkCreate()
        {}
        public virtual void OnServerSend(UInt64 stream_handle)
        {}
        
        public virtual void OnServerReceive(UInt64 stream_handle)
        {}
        public virtual void OnClientReceive(UInt64 stream_handle)
        {}
        
        public virtual void OnClientSend(UInt64 stream_handle)
        {}

        public bool IsOwner()
        {
            return InternalCalls.Action_IsOwner(Id);
        }

        // -----------------------------------------------------


    }
}
