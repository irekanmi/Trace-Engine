#pragma once


#include "core/io/Logging.h"

#include <vector>


namespace trace {


    template<typename T>
    class RingBuffer
    {


    public:

        // Constructor to intialize circular buffer's data
        // members
        RingBuffer(int _capacity) 
        {

            // If the capacity is invalid
            if (_capacity < 0) 
            {
                TRC_ASSERT(_capacity > 0, "Invalid capacity");
            }
            this->capacity = _capacity + 1;
            this->front = 0;
            this->back = 0;
            buffer.resize(capacity);
        }

        // Function to add an element to the buffer
        void push_back(T val) 
        {
            if (full()) 
            {
                TRC_ASSERT(!full(), "RingBuffer is full");
            }
            buffer[back] = val;
            back = (back + 1) % capacity;
        }

        // Function to remove an element from the buffer
        void pop_front() 
        {
            if (empty()) 
            {
                TRC_ASSERT(!empty(), "RingBuffer is empty");
            }
            front = (front + 1) % capacity;
        }

        int32_t getFront() 
        {
            if (empty()) 
            {
                TRC_ASSERT(!empty(), "RingBuffer is empty");
            }
            return buffer[front];
        }

        int32_t getBack() 
        {
            if (empty()) 
            {
                TRC_ASSERT(!empty(), "RingBuffer is empty");
            }
            return (back == 0) ? buffer[capacity - 1] : buffer[back - 1];
        }

        // Function to check if the buffer is empty
        bool empty() const { return front == back; }

        // Function to check if the buffer is full
        bool full() const 
        {
            return (back + 1) % capacity == front;
        }

        // Function to get the size of the buffer
        int size() const 
        {
            if (back >= front) 
            {
                return back - front;
            }
            return capacity - (front - back);
        }

        // Function to print the elements of the buffer
        void iterate(std::function<void(T&)> callback)
        {
            if (back > front)
            {
                for (int32_t i = 0; i < size(); i++)
                {
                    int32_t index = i + front;
                    callback(buffer[index]);
                }
            }
            else if (back < front)
            {
                int32_t index = front;
                while (index != back)
                {
                    callback(buffer[index]);
                    index = (index + 1) % capacity;
                }
                callback(buffer[back]);
            }
        }

    private:
        std::vector<T> buffer;
        int32_t front;
        int32_t back;
        int32_t capacity;

    };

}
