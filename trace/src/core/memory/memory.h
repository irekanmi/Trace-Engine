#pragma once

#include "../pch.h"
#include "../Enums.h"
#include "../io/Logging.h"


namespace trace {

	// TODO: change pool allocator to none template pool
	template<typename T, size_t ChunksPerBlock = BLOCK_SIZE>
	class MemoryPoolLL
	{
		static_assert(sizeof(T) >= sizeof(void*), "can't allocate for type less than sizeof void*");
	public:

		MemoryPoolLL()
		{

		}

		~MemoryPoolLL()
		{
			Clean();
		}

		T* Malloc()
		{
			if (head == nullptr)
			{
				head = allocate(nullptr);
			}

			chunk_t* ret = head;
			head = ret->m_next;

			return reinterpret_cast<T*>(ret);
		}

		void Free(void* ptr)
		{
			chunk_t* val = reinterpret_cast<chunk_t*>(ptr);
			val->m_next = head;
			head = val;
		}

		void Clean()
		{
			if (!cleaned)
			{
				for (block_t::iterator it = p_blocks.begin(); it != p_blocks.end(); it++)
				{
					auto val = *it;
					std::free(val);
				}
				cleaned = true;
			}
		}

		void Reserve(size_t num_blocks)
		{
			chunk_t* ptr = nullptr;
			for (size_t i = 0; i < num_blocks; i++)
			{
				ptr = allocate(ptr);
			}
			head = ptr;
		}

	private:
		struct chunk_t
		{
			chunk_t(chunk_t* next)
			{
				m_next = next;
			}

			union
			{
				chunk_t* m_next;
				unsigned char m_data[sizeof(T)];
			};
		};

		using block_t = std::vector<void*>;

		block_t p_blocks;
		size_t chunk_per_block = ChunksPerBlock;
		chunk_t* head = nullptr;
		bool cleaned = false;
	private:

		chunk_t* allocate(chunk_t* tail)
		{

			chunk_t* block = reinterpret_cast<chunk_t*>(std::malloc(sizeof(chunk_t) * chunk_per_block));

			if (block)
			{
				p_blocks.push_back(block);
				chunk_t* chunk = block;

				for (size_t i = 0; i < chunk_per_block - 1; i++)
				{
					(*chunk) = chunk_t(chunk + 1);
					chunk = chunk->m_next;
				}
				(*chunk) = chunk_t(tail);
				return block;
			}
			else
			{
				TRC_CRITICAL("unable to allocate block %p", block)
				return nullptr;
			}

		}


	

	};
}
