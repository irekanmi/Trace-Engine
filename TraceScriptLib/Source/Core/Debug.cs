using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    static public class Debug
    {
        public static void Log(string message)
        {
            InternalCalls.Debug_Log(message);
        }

        public static void Trace(string message)
        {
            InternalCalls.Debug_Trace(message);
        }

        public static void Info(string message)
        {
            InternalCalls.Debug_Info(message);
        }

    }
}
