#pragma once

#include "multithreading/SpinLock.h"

#include <array>
#include <cinttypes>
#include <mutex>


namespace trace {

	template<typename T, uint32_t buffer_size>
	class ThreadRingBuffer
	{

	public:

		void push_front(T& val)
		{
			std::lock_guard<SpinLock> guard(m_writeLock);
			uint32_t index = tail;
			m_data[index] = val;
			tail = (tail + 1) % buffer_size;
		}

		void push_back(T& val)
		{
			std::lock_guard<SpinLock> guard(m_readLock);
			head = (head - 1) % buffer_size;
			uint32_t index = head;
			m_data[index] = val;
		}

		T* pop_back()
		{
			std::lock_guard<SpinLock> guard(m_readLock);


			if (head == tail)
			{
				return nullptr;
			}

			uint32_t index = head;
			head = (head + 1) % buffer_size;
			return &m_data[index];
		}

	private:
		std::array<T, buffer_size> m_data;
		SpinLock m_writeLock;
		SpinLock m_readLock;

		std::atomic<uint32_t> head = 0;
		std::atomic<uint32_t> tail = 0;

	protected:

	};
}
