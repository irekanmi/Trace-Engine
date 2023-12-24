using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Action
    {
        public readonly ulong Id;
        protected Action() { Id = 0; }

        internal protected Action(ulong id)
        {
            Id = id;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>()) return null;
            Type component_type = typeof(T);
            object component = InternalCalls.Action_GetComponent(Id, component_type); ;
            return component as T;
        }

        public T GetScript<T>() where T : Action
        {
            if (!HasScript<T>()) return null;
            Type script_type = typeof(T);
            object script = InternalCalls.Action_GetScript(Id, script_type);
            return script as T;
        }

        public bool HasComponent<T>() where T : Component
        {
            Type component_type = typeof(T);
            return InternalCalls.Action_HasComponent(Id, component_type);
        }

        public bool HasScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            return InternalCalls.Action_HasScript(Id, script_type);
        }

        public T AddComponent<T>() where T : Component, new()
        {
            Type component_type = typeof(T);
            return null;
        }

        public T AddScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            return null;
        }

        public void RemoveComponent<T>() where T : Component, new()
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

    }
}
