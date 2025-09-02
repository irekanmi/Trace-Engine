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

        static public float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }
    }

    public class RingBuffer<T>
    {
        public T[] data;
        private int size;
        private int front = 0;
        private int back = 0;

        public RingBuffer(int capacity)
        {
            data = new T[capacity];
            size = capacity;
        }

        public int Count
        {
            get
            {
                if(back >= front)
                {
                    return back - front;
                }

                return size - (front - back);
            }
        }

        public bool Full
        {
            get
            {
                return ((back + 1) % size) == front;
            }
            
        }

        public bool Empty
        {
            get
            {
                return Count == 0;
            }
        }

        public void PushBack(T value)
        {
            if(Full)
            {
                Debug.Log("Ring Buffer is full try to remove an element");
                return;
            }

            data[back] = value;
            back = (back + 1) % size;
        }

        public T PopFront()
        {
            if(Empty)
            {
                return default(T);
            }
            T value = data[front];
            front = (front + 1) % size;
            return value;
        }

        public bool Iterate(int current_index, out T index_value)
        {
            index_value = data[0];
            if(Empty)
            {
                return false;
            }

            if(back > front)
            {
                int index = front + current_index;
                if(index >= back)
                {
                    return false;
                }

                index_value = data[index];
            }
            else
            {
                int index = (front + current_index) % size;
                if(index >= back  && index < front)
                {
                    return false;
                }

                index_value = data[index];
            }

            return true;
        }

        public T Get(int index)
        {
            T value = data[0];
            if (back > front)
            {
                int actual_index = front + index;
                if (actual_index >= back)
                {
                    Debug.Log("RingBuffer.Get(index), Invalid index");
                }

                value = data[actual_index];
            }
            else
            {
                int actual_index = (front + index) % size;
                if (actual_index >= back && actual_index < front)
                {
                    Debug.Log("RingBuffer.Get(index), Invalid index");
                }

                value = data[actual_index];
            }

            return value;

        }



    }
}
