#pragma once

#include "core/FileSystem.h"

#include <string>

namespace trace {


	class FileStream
	{

	public:
		FileStream(const std::string& file_path, FileMode mode);
		~FileStream();

		void Read(void* data, uint32_t size);
		void Write(void* data, uint32_t size);
		void SetPosition(uint32_t pos);
		uint32_t GetPosition() { return m_pos; }

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
