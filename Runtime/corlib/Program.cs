﻿using System;
using System.Runtime.CompilerServices;

namespace Mernel
{
    internal static class Program
    {
        private static void Main()
        {
            //Console.WriteLine("Mernel: Startup");
            string str = "Are you sure?";
            string str2 = "Definately.";
            //str2 = str2.Replace("fin", "fan");
            //str2 = str2.Replace("Definately", "Fosho!");
            // Console.WriteLine(str + "\n" + str2);
            int a = 2;
            int b = 78;
            //b = 2;
            //a = aVal(b);
            Console.WriteLine("A Five times is: \"" + Factorial(5, "") + "\"");
            if (a < b) 
                Console.WriteLine("It is!");
            else
                Console.WriteLine("It Isn't!");
            //Console.WriteLine((string)((int)1).ToString());
        }

        private static string Factorial(int val, string str)
        {
            return val == 0 ? str : str + Factorial(val - 1, str + " a");
        }
    }
}
