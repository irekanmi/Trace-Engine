#include "pch.h"

#include "MemoryManager.h"
#include "core/Enums.h"

namespace trace {

	MemoryManager* MemoryManager::s_instance = nullptr;


	bool MemoryManager::Init()
	{
		bool result = true;
		result = result && m_frameAllocator.Init(KB * 8); // TODO: Configurable
		return result;
	}

	void MemoryManager::Shutdown()
	{
		m_frameAllocator.Shutdown();
	}

	bool MemoryManager::BeginFrame()
	{
		m_frameAllocator.Clear();
		return true;
	}

	void MemoryManager::EndFrame()
	{
		return;
	}

	void* MemoryManager::FrameAlloc(uint32_t num_bytes)
	{
		return m_frameAllocator.Push(num_bytes);
	}

	MemoryManager* MemoryManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new MemoryManager;
		}
		return s_instance;
	}

}