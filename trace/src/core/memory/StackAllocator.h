#pragma once

#include <cstdint>

namespace trace {

	class StackAllocator
	{

	public:

		bool Init(uint32_t num_bytes);
		void Shutdown();

		void* Push(uint32_t num_bytes);
		void Pop();
		void Clear();

	private:
		unsigned char* m_data = nullptr;
		uint32_t m_numBytes = 0;
		uint32_t m_currMark = 0;
	protected:
		

	};

}
