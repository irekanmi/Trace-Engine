#pragma once

#include <stdint.h>

namespace trace {

	class MemoryStream
	{

	public:
		MemoryStream(void* data, uint32_t size,bool read = true);
		~MemoryStream();

		void Read(void* data, uint32_t size);
		void Write(void* data, uint32_t size);
		void SetPosition(uint32_t pos);
		uint32_t GetPosition() { return m_pos; }
		char* GetData() { return m_data; }

		template<typename T>
		void Read(T& value)
		{
			Read(&value, sizeof(T));
		}

		template<typename T>
		void Write(T& value)
		{
			Write(&value, sizeof(T));
		}

	private:
		char* m_data;
		uint32_t m_pos;
		uint32_t m_size;
		bool m_readMode;


	protected:

	};

}