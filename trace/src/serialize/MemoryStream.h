#pragma once

#include "serialize/DataStream.h"

#include <stdint.h>

namespace trace {

	class MemoryStream : public DataStream
	{

	public:
		MemoryStream(void* data, uint32_t size, bool read = true);
		~MemoryStream();

		virtual void Read(void* data, uint32_t size) override;
		virtual void Write(void* data, uint32_t size) override;
		virtual void SetPosition(uint32_t pos) override;
		virtual uint32_t GetPosition() override { return m_pos; }
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