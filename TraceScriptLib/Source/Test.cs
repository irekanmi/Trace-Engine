using System;
using System.Runtime.CompilerServices;

namespace Trace
{
    public class Test
    {

        public Test()
        {
            Console.WriteLine("Test Object Constructor Called");
            SayWelcome();
        }

        public void SayHello()
        {
            LogString("Coker Ayanfe Irekanmi");
        }

        private void SayWelcome()
        {
            Console.WriteLine("Welcome to C#");
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void LogString(string text);

    }    

}
