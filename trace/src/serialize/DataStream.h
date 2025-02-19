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

	private:
	protected:

	};





}
