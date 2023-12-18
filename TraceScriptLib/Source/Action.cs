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
        public Action() { Id = 0; }

        public Action(ulong id)
        {
            Id = id;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>()) return null;
            Type component_type = typeof(T);
            return null;
        }

        public T GetScript<T>() where T : Action
        {
            if (!HasScript<T>()) return null;
            Type script_type = typeof(T);
            return null;
        }

        public bool HasComponent<T>() where T : Component
        {
            Type component_type = typeof(T);
            return false;
        }

        public bool HasScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            return false;
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
            return;
        }

        public void RemoveScript<T>() where T : Action
        {
            Type script_type = typeof(T);
            return;
        }

    }
}
