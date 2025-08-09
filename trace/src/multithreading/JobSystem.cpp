#include "pch.h"

#include "multithreading/JobSystem.h"
#include "core/io/Logging.h"

namespace trace {


	static void worker_thread_func()
	{
		JobSystem* job_system = JobSystem::get_instance();
		// Initialization .....
		ThreadInfo& _info = job_system->GetThreadInfo()[job_system->GetThreadID()];
		_info.supported_jobs |= JobFlagBit::GENERAL;


		// Loop.....
		while (job_system->IsActive())
		{
			if (job_system->TryGetJob())
			{
				Job job = _info.current_job;
				Counter* job_counter = job.counter;
				job.job_func(job.job_params);
				if (job_counter)
				{
					job_counter->Decrement();
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}

		}

	}

	bool JobSystem::Init()
	{
		uint32_t num_threads = std::thread::hardware_concurrency();
		m_numThreads = num_threads;
		num_threads -= 1;// remove main thread

		const uint32_t num_flags = 8;
		JobFlag thread_flags[num_flags] = {0};

		ThreadInfo& _info = m_threadInfo[GetThreadID()];

		if (num_threads <= 1)
		{
			_info.supported_jobs |= JobFlagBit::GENERAL | JobFlagBit::ASYNC_IO | JobFlagBit::AUDIO | JobFlagBit::DEBUG | JobFlagBit::NETWORK;
			thread_flags[0] = JobFlagBit::RENDER | JobFlagBit::ASYNC_IO;

		}
		else if (num_threads == 2)
		{
			thread_flags[0] = JobFlagBit::RENDER | JobFlagBit::DEBUG | JobFlagBit::ASYNC_IO;
			thread_flags[1] = JobFlagBit::NETWORK | JobFlagBit::ASYNC_IO;
			_info.supported_jobs |= JobFlagBit::AUDIO;

		}
		else if (num_threads == 3)
		{
			thread_flags[0] = JobFlagBit::RENDER;
			thread_flags[1] = JobFlagBit::NETWORK | JobFlagBit::ASYNC_IO;
			thread_flags[2] = JobFlagBit::AUDIO | JobFlagBit::ASYNC_IO;

		}
		else if (num_threads == 4)
		{
			thread_flags[0] = JobFlagBit::RENDER;
			thread_flags[1] = JobFlagBit::NETWORK | JobFlagBit::ASYNC_IO;
			thread_flags[2] = JobFlagBit::AUDIO;
			thread_flags[3] = JobFlagBit::ASYNC_IO;

		}
		else if (num_threads > 4)
		{
			thread_flags[0] = JobFlagBit::RENDER;
			thread_flags[1] = JobFlagBit::NETWORK;
			thread_flags[2] = JobFlagBit::AUDIO;
			thread_flags[3] = JobFlagBit::ASYNC_IO;
			thread_flags[4] = JobFlagBit::ASYNC_IO;
			thread_flags[5] = JobFlagBit::ASYNC_IO;
			thread_flags[6] = JobFlagBit::ASYNC_IO;
			thread_flags[7] = JobFlagBit::ASYNC_IO;

		}

		_info.supported_jobs |= JobFlagBit::GENERAL;


		for (uint32_t i = 0; i < num_threads; i++)
		{
			m_threadPool.emplace_back([i, thread_flags, num_flags, this]() 
			{
				if(i < num_flags)
				{
					JobFlag thread_flag = thread_flags[i];
					ThreadInfo& _info = this->GetThreadInfo()[this->GetThreadID()];
					_info.supported_jobs |= thread_flag;
				}
				worker_thread_func();
			});
		}

		m_active = true;

		return true;
	}

	void JobSystem::Shutdown()
	{
		m_active = false;

		for (auto& t : m_threadPool)
		{
			t.join();
		}

	}

	void JobSystem::RunJob(Job job, Counter* counter)
	{
		if (counter)
		{
			counter->Increment();
		}
		job.counter = counter;

		m_jobs.push_front(job);
	}

	void JobSystem::WaitForCounter(Counter* counter)
	{
		if (!counter)
		{
			std::cout << "Pass in a valid counter pointer, " << "Function: " << __FUNCTION__ << std::endl;
			return;
		}

		ThreadInfo& _info = m_threadInfo[GetThreadID()];
		while (!counter->IsFree())
		{
			// check if counter is free a few more times
			uint32_t num_tries = 12;//TODO: Configurable or change the alogrithim
			while (num_tries > 0)
			{
				if (counter->IsFree())
				{
					return;
				}

				--num_tries;
			}

			//Try to run another job while waiting for counter
			if (TryGetJob())
			{
				Job job = _info.current_job;
				Counter* job_counter = job.counter;
				job.job_func(job.job_params);
				if (job_counter)
				{
					job_counter->Decrement();
				}
			}
			else
			{
				// sleep thread
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

		}
	}

	void JobSystem::WaitForCounterAndFree(Counter* counter)
	{
		WaitForCounter(counter);
		if (counter)
		{
			delete counter;//TODO: Use custom allocator
			counter = nullptr;
		}
	}

	Counter* JobSystem::CreateCounter()
	{
		return new Counter{};// TODO: Use Custom Allocator
	}

	bool JobSystem::TryGetJob()
	{
		ThreadInfo& _info = m_threadInfo[GetThreadID()];
		uint32_t thread_flags = _info.supported_jobs;

		Job* new_job = nullptr;

		// try to get a job few times
		uint32_t num_tries = 12;//TODO: Configurable or change the alogrithim
		while (num_tries > 0)
		{
			Job* _job = m_jobs.pop_back();
			if (_job)
			{
				uint32_t job_flag = _job->flags;
				if ((thread_flags & job_flag) == job_flag)
				{
					new_job = _job;
					break;
				}
				else
				{
					// return job to job pool if it can run on this thread
					m_jobs.push_back(*_job);
				}
			}

			--num_tries;
		}

		if (new_job)
		{
			_info.current_job = *new_job;
			return true;
		}


		return false;
	}

	size_t JobSystem::GetThreadID()
	{
		std::hash<std::thread::id> hasher;
		size_t _id = hasher(std::this_thread::get_id());
		return _id;
	}

	uint32_t JobSystem::GetThreadCount()
	{
		return m_numThreads;
	}

	JobSystem* JobSystem::get_instance()
	{
		static JobSystem* s_instance = new JobSystem;
		return s_instance;
	}

	void Counter::Increment()
	{
		std::lock_guard<SpinLock> guard(lock);

		index.fetch_add(1);
	}

	void Counter::Decrement()
	{
		std::lock_guard<SpinLock> guard(lock);

		index.fetch_sub(1);
	}

	bool Counter::IsFree()
	{
		uint32_t value = index.load();

		return value == 0;
	}


}