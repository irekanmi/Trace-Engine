#pragma once

#include <atomic>
#include <emmintrin.h> // _mm_pause

namespace trace {


	class SpinLock
	{

	public:

		bool TryAcquire()
		{
			bool acquired = m_lock.test_and_set(std::memory_order::memory_order_acquire);

			return !acquired;
		}

		void Acquire()
		{
			while (!TryAcquire())
			{
				_mm_pause();
			}
		}

		void Release()
		{
			m_lock.clear(std::memory_order::memory_order_release);
		}

		void Lock() { Acquire(); }
		void Unlock() { Release(); }
		void lock() { Acquire(); }
		void unlock() { Release(); }


	private:
		std::atomic_flag m_lock;

	protected:

	};

}
