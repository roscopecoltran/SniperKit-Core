using System;
using System.Runtime.InteropServices;

namespace Assets.Scripts.Core.Interop
{
    internal static class MarshalUtils
    {
        public static double[] ReadDoubles(IntPtr ptr, int size)
        {
            var doubles = new double[size];
            Marshal.Copy(ptr, doubles, 0, size);
            return doubles;
        }

        public static int[] ReadInts(IntPtr ptr, int size)
        {
            var ints = new int[size];
            Marshal.Copy(ptr, ints, 0, size);
            return ints;
        }

        public static string[] ReadStrings(IntPtr ptr, int size)
        {
            var strings = new string[size];
            var address = IntPtr.Size == 4 ? ptr.ToInt32() : ptr.ToInt64();
            for (int i = 0; i < size; ++i)
            {
                ptr = new IntPtr(address + IntPtr.Size * i);
                // TODO Not working with non-latin symbols
                strings[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(ptr));
            }
            return strings;
        }
    }
}
