#pragma once

#include "multithreading/ThreadRingBuffer.h"
#include "core/Enums.h"

#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>

#define JOB_POOL_SIZE KB

namespace trace {

	class Event;

	enum JobFlagBit
	{
		UNKNOWN,
		AUDIO = BIT(1),
		ASYNC_IO = BIT(2),
		GENERAL = BIT(3),
		DEBUG = BIT(4),
		RENDER = BIT(5),
		NETWORK = BIT(6),
	};

	using JobFlag = uint32_t;

	struct Counter;
	struct Job
	{
		std::function<void(void*)> job_func;
		JobFlag flags;
		Counter* counter;// Job Sync Primitive
		void* job_params = nullptr;
	};

	struct ThreadInfo
	{
		size_t thread_id = 0;
		JobFlag supported_jobs = 0;
		Job current_job;
	};

	struct Counter
	{
		std::atomic<int32_t> index = 0;
		SpinLock lock;

		void Increment();
		void Decrement();
		bool IsFree();

	};

	class JobSystem
	{

	public:

		bool Init();
		void Shutdown();

		void RunJob(Job job, Counter* counter = nullptr);
		void WaitForCounter(Counter* counter);
		void WaitForCounterAndFree(Counter* counter);
		bool IsActive() { return m_active; }
		bool AppRunning() { return !m_appShutdown; }

		Counter* CreateCounter();

		std::unordered_map<size_t, ThreadInfo>& GetThreadInfo() { return m_threadInfo; }
		bool TryGetJob();
		size_t GetThreadID();
		uint32_t GetThreadCount();
		void OnEvent(Event* p_event);

		static JobSystem* get_instance();
	private:
		std::vector<std::thread> m_threadPool;
		std::unordered_map<size_t, ThreadInfo> m_threadInfo;
		ThreadRingBuffer<Job, JOB_POOL_SIZE> m_jobs;
		bool m_active = false;
		bool m_appShutdown = false;
		uint32_t m_numThreads;


	protected:

	};

}