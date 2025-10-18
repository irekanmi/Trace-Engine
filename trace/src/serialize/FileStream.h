#pragma once

#include "core/FileSystem.h"
#include "serialize/DataStream.h"

#include <string>

namespace trace {


	class FileStream : public DataStream
	{

	public:
		FileStream(const std::string& file_path, FileMode mode);
		~FileStream();

		virtual void Read(void* data, uint32_t size) override;
		virtual void Write(void* data, uint32_t size) override;
		virtual void SetPosition(uint32_t pos) override;
		virtual uint32_t GetPosition() override { return m_pos; }

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
		FileHandle m_file;
		uint32_t m_pos;
		bool m_readMode;


	protected:

	};

	



}
