#pragma once

#include "serialize/DataStream.h"

#include <string>

namespace trace::Network {


	class NetworkStream : public DataStream
	{

	public:
		NetworkStream(void* data, uint32_t size, bool copy = false);
		NetworkStream(uint32_t size);
		NetworkStream();
		NetworkStream(NetworkStream& other);
		~NetworkStream();

		virtual void Read(void* data, uint32_t size) override;
		virtual void Write(void* data, uint32_t size) override;
		virtual void SetPosition(uint32_t pos) override;
		virtual uint32_t GetPosition() override;
		void Write(uint32_t position, void* data, uint32_t size);
		uint32_t GetSize() { return m_size; };

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
		
		template<typename T>
		void Write(uint32_t position, T& value)
		{
			Write(position, &value, sizeof(T));
		}

		template<>
		void Read<std::string>(std::string& data);

		template<>
		void Write<std::string>(std::string& data);

		void MemSet(uint32_t start_position, uint32_t end_position, uint32_t data);
		void operator =(NetworkStream& other);

		void Destroy();
		void* GetData() { return m_data; }

	private:
		void allocate_mem(uint32_t size);

	private:
		void* m_data = nullptr;
		uint32_t m_size = 0;
		uint32_t m_memSize = 0;
		uint32_t m_position = 0;
		bool m_copied = false;
		bool m_destroyed = false;
	protected:

	};

	template<>
	inline void NetworkStream::Read(std::string& data)
	{
		uint32_t size = 0;
		Read(size);
		data.resize(size);
		Read(data.data(), size);
	}

	template<>
	inline void NetworkStream::Write(std::string& data)
	{
		uint32_t size = static_cast<uint32_t>(data.size());
		Write(size);
		Write(data.data(), size);
	}

}
