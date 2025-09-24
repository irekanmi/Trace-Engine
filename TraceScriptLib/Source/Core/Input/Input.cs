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

        static public bool GetKeyPressed(Keys key_code)
        {
            return InternalCalls.Input_GetKeyPressed(key_code);
        }

        static public bool GetKeyReleased(Keys key_code)
        {
            return InternalCalls.Input_GetKeyReleased(key_code);
        }

        static public bool GetGamepadKey(GamepadKeys key_code, int controller_id  = 0)
        {
            return InternalCalls.Input_GetGamepadKey(key_code, controller_id);
        }

        static public bool GetGamepadKeyPressed(GamepadKeys key_code, int controller_id = 0)
        {
            return InternalCalls.Input_GetGamepadKeyPressed(key_code, controller_id);
        }

        static public bool GetGamepadKeyReleased(GamepadKeys key_code, int controller_id = 0)
        {
            return InternalCalls.Input_GetGamepadKeyReleased(key_code, controller_id);
        }

        static public float GetLeftStickX(int controller_id = 0)
        {
            return InternalCalls.Input_GetLeftStickX(controller_id);
        }
        static public float GetLeftStickY(int controller_id = 0)
        {
            return InternalCalls.Input_GetLeftStickY(controller_id);
        }
        
        static public float GetRightStickX(int controller_id = 0)
        {
            return InternalCalls.Input_GetRightStickX(controller_id);
        }
        static public float GetRightStickY(int controller_id = 0)
        {
            return InternalCalls.Input_GetRightStickY(controller_id);
        }
        
        static public float GetLeftTrigger(int controller_id = 0)
        {
            return InternalCalls.Input_GetLeftTrigger(controller_id);
        }
        static public float GetRightTrigger(int controller_id = 0)
        {
            return InternalCalls.Input_GetRightTrigger(controller_id);
        }

        static public bool GetButton(Buttons button_code)
        {
            return InternalCalls.Input_GetButton(button_code);
        }

        static public bool GetButtonPressed(Buttons button_code)
        {
            return InternalCalls.Input_GetButtonPressed(button_code);
        }

        static public bool GetButtonReleased(Buttons button_code)
        {
            return InternalCalls.Input_GetButtonReleased(button_code);
        }

    }
}
