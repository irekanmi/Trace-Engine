#include "pch.h"

#include "MemoryStream.h"
#include "io/Logging.h"

namespace trace {



	MemoryStream::MemoryStream(void* data,uint32_t size, bool read)
	{
		m_readMode = read;
		m_data = (char*)data;
		m_pos = 0;
		m_size = size;
	}

	MemoryStream::~MemoryStream()
	{
		m_data = nullptr;
	}

	void MemoryStream::Read(void* data, uint32_t size)
	{
		if (!m_data)
		{
			TRC_WARN("Invalid Memory Stream");
			return;
		}

		if (!m_readMode)
		{
			TRC_WARN("This memory stream is not open for reading, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		if ((m_pos + size) > m_size)
		{
			TRC_WARN("Unavaliable stream size");
			return;
		}

		memcpy(data, (m_data + m_pos), size);

		m_pos += size;
	}

	void MemoryStream::Write(void* data, uint32_t size)
	{
		if (!m_data)
		{
			TRC_WARN("Invalid Memory Stream");
			return;
		}

		if (m_readMode)
		{
			TRC_WARN("This Memory stream is not open for writing, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		if ((m_pos + size) > m_size)
		{
			TRC_WARN("Unavaliable stream size");
			return;
		}

		memcpy((m_data + m_pos), data, size);
		
		m_pos += size;
	}

	void MemoryStream::SetPosition(uint32_t pos)
	{
		if (pos > m_size)
		{
			TRC_WARN("Unavaliable stream size");
			return;
		}
		m_pos = pos;
	}

}