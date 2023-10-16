#include "pch.h"
#include "StackAllocator.h"
#include "core/io/Logging.h"

namespace trace {

	bool StackAllocator::Init(uint32_t num_bytes)
	{
		TRC_ASSERT(num_bytes != 0, "Can't create a stack with zero(0) bytes");

		m_numBytes = num_bytes;
		m_currMark = 0;
		m_data = new unsigned char[m_numBytes];
		if (!m_data)
		{
			TRC_ERROR("Unable to allocate memory {}", __FUNCTION__); 
			return false;
		}
		return true;
	}

	void StackAllocator::Shutdown()
	{
		if (m_data)
		{
			delete[] m_data; 
			m_data = nullptr;
			m_currMark = 0;
		}
	}

	void* StackAllocator::Push(uint32_t num_bytes)
	{
		if (!m_data) return nullptr;

		uint32_t next_mark = m_currMark + num_bytes;
		if (next_mark + 4 >= m_numBytes)
		{
			TRC_ERROR("Stack out of memory, -> {}", (const void*)m_data);
			return nullptr;
		}
		unsigned char* result = (m_data) + m_currMark;
		uint32_t& prev_mark = *((uint32_t*)(m_data + next_mark));
		prev_mark = m_currMark;
		m_currMark = next_mark + 4;

		return result;
	}

	void StackAllocator::Pop()
	{
		if (!m_data || m_currMark == 0) return;
		uint32_t& prev_mark = *((uint32_t*)(m_data + (m_currMark - 4)));
		m_currMark = prev_mark;
	}

	void StackAllocator::Clear()
	{
		m_currMark = 0;
	}


}


