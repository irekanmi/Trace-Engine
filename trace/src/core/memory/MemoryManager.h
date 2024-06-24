#pragma once

#include "StackAllocator.h"


namespace trace {

	class MemoryManager
	{

	public:

		bool Init();
		void Shutdown();

		bool BeginFrame();
		void EndFrame();
		void* FrameAlloc(uint32_t num_bytes);

		static MemoryManager* get_instance();
	private:

		StackAllocator m_frameAllocator;

	protected:


	};

}
