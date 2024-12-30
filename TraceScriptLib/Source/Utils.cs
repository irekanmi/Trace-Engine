using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Utils
    {
        static public ulong HashString(string str)
        {
            ulong hash_value = 0xcbf29ce484222325UL;
            ulong prime = 0x100000001b3UL;
            foreach (char c in str)
            {
                hash_value ^= Convert.ToUInt64(c);
                hash_value *= prime;
            }
            return hash_value;
        }
    }
}
