using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public static class Application
    {

        public static System.Action<int> on_key_typed;

        public static bool LoadAndSetScene(string scene_filename)
        {
            return InternalCalls.Application_LoadAndSetScene(scene_filename);
        }

        private static void InvokeKeyTyped(int letter)
        {
            if (on_key_typed != null)
            {
                on_key_typed(letter);
            }
        }

    }
}
