#pragma once


#include <string>

namespace trace {


	class DataStream
	{

	public:

		virtual void Read(void* data, uint32_t size) = 0;
		virtual void Write(void* data, uint32_t size) = 0;
		virtual void SetPosition(uint32_t pos) = 0;
		virtual uint32_t GetPosition() = 0;

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

		template<>
		void Read(std::string& value)
		{
			uint32_t size = 0;
			Read<uint32_t>(size);
			value.resize(size);
			Read(value.data(), size);
		}

		template<>
		void Write(std::string& val)
		{
			uint32_t size = static_cast<uint32_t>(val.size());
			Write<uint32_t>(size);
			Write(val.data(), size);
		}

	private:
	protected:

	};





}
