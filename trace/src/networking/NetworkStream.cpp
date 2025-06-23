#include "pch.h"

#include "networking/NetworkStream.h"
#include "core/io/Logging.h"

namespace trace::Network {
	NetworkStream::NetworkStream(void* data, uint32_t size, bool copy)
	{
		if (copy)
		{
			m_copied = true;
			m_destroyed = false;
			allocate_mem(size);
			memcpy(m_data, data, size);
			m_size = size;
			m_position = 0;
		}
		else
		{
			m_copied = false;
			m_destroyed = true;
			m_data = data;
			m_size = size;
			m_position = 0;
		}
	}

	NetworkStream::NetworkStream(uint32_t size)
	{
		m_copied = true;
		m_destroyed = false;
		allocate_mem(size);
		m_size = size;
		m_position = 0;
	}

	NetworkStream::NetworkStream()
	{
		m_copied = false;
		m_destroyed = true;
		m_data = nullptr;
		m_size = 0;
		m_position = 0;
	}

	NetworkStream::NetworkStream(NetworkStream& other)
	{
		m_copied = true;
		m_destroyed = false;
		allocate_mem(other.m_size);
		memcpy(m_data, other.m_data, other.m_size);
		m_size = other.m_size;
		//m_position = 0;
		m_position = other.m_position;
	}

	NetworkStream::~NetworkStream()
	{
		Destroy();
	}


	void NetworkStream::Read(void* data, uint32_t size)
	{
		if ((m_position + size) > m_size)
		{
			TRC_ERROR("Reached end of data stream, Function: {}", __FUNCTION__ );
			return;
		}

		unsigned char* location = (unsigned char*)m_data + m_position;
		memcpy(data, location, size);

		m_position += size;
	}

	void NetworkStream::Write(void* data, uint32_t size)
	{
		if ((m_position + size) > m_size)
		{
			if(m_copied)
			{ 
				uint32_t new_size = m_size * 2;
				char* new_data = new char[size];//TODO: Use custom allocator
				memcpy(new_data, m_data, m_size);
				m_memSize = new_size;
				m_size = new_size;
				delete[] m_data;//TODO: Use custom allocator
				m_data = new_data;
			}
			else
			{
				TRC_WARN("Reached end of data stream, Function: {}", __FUNCTION__);
				return;
			}
		}

		unsigned char* location = (unsigned char*)m_data + m_position;
		memcpy(location, data, size);

		m_position += size;
	}

	void NetworkStream::SetPosition(uint32_t pos)
	{
		if (pos < m_size)
		{
			m_position = pos;
		}
	}

	void NetworkStream::Write(uint32_t position, void* data, uint32_t size)
	{
		uint32_t curr_pos = GetPosition();
		SetPosition(position);
		Write(data, size);
		SetPosition(curr_pos);
	}

	void NetworkStream::MemSet(uint32_t start_position, uint32_t end_position, uint32_t data)
	{
		unsigned char* location = (unsigned char*)m_data + start_position;
		uint32_t size = end_position - start_position;
		memset(location, data, size);
	}

	void NetworkStream::Destroy()
	{
		if (m_copied && !m_destroyed)
		{
			delete[] m_data;//TODO: Use custom allocator
			m_destroyed = true;
		}
	}

	uint32_t NetworkStream::GetPosition()
	{
		return m_position;
	}

	void NetworkStream::operator=(NetworkStream& other)
	{
		if (m_data)
		{
			if (m_memSize >= other.m_size)
			{
				memset(m_data, 0, m_size);
				memcpy(m_data, other.m_data, other.m_size);
				m_position = other.m_position;
				m_size = other.m_size;
			}
			else
			{
				delete[] m_data;
				allocate_mem(other.m_size);
				memcpy(m_data, other.m_data, other.m_size);
				m_position = other.m_position;
				m_size = other.m_size;
			}
		}
		else
		{
			m_copied = true;
			m_destroyed = false;
			allocate_mem(other.m_size);
			memcpy(m_data, other.m_data, other.m_size);
			m_size = other.m_size;
			//m_position = 0;
			m_position = other.m_position;
		}
	}

	void NetworkStream::allocate_mem(uint32_t size)
	{
		m_data = new char[size];//TODO: Use custom allocator
		m_memSize = size;
	}

}