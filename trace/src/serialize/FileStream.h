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

	private:
		FileHandle m_file;
		uint32_t m_pos;
		bool m_readMode;


	protected:

	};

	



}
