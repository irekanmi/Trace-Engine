using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Trace
{
    public class Input
    {

        static public bool GetKey(Keys key_code)
        {
            return InternalCalls.Input_GetKey(key_code);
        }

        static public bool GetButton(Buttons button_code)
        {
            return InternalCalls.Input_GetButton(button_code);
        }

    }
}
