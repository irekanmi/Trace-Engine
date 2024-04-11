#include "pch.h"

#include "FileStream.h"
#include "io/Logging.h"

namespace trace {



	FileStream::FileStream(const std::string& file_path, FileMode mode)
	{
		m_readMode = (mode == FileMode::READ);
		m_pos = 0;

		if (!FileSystem::open_file(file_path, (FileMode)(mode | FileMode::BINARY), m_file))
		{
			TRC_ERROR("Unable to open file -> {}, failed to create file stream", file_path);
			return;
		}

	}

	FileStream::~FileStream()
	{
		FileSystem::close_file(m_file);
	}

	void FileStream::Read(void* data, uint32_t size)
	{
		if (!m_readMode)
		{
			TRC_WARN("This file stream is not open for reading, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		if (!m_file.m_isVaild)
		{
			TRC_WARN("This file stream is not valid, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		FileSystem::read(data, size, m_file);

		m_pos += size;
	}

	void FileStream::Write(void* data, uint32_t size)
	{

		if (m_readMode)
		{
			TRC_WARN("This file stream is not open for writing, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		if (!m_file.m_isVaild)
		{
			TRC_WARN("This file stream is not valid, Line -> {}, Funtion -> {}", __LINE__, __FUNCTION__);
			return;
		}

		FileSystem::write(data, size, m_file);
		m_pos += size;
	}

	void FileStream::SetPosition(uint32_t pos)
	{
		FileSystem::set_seek_position(m_file, pos);
		m_pos = pos;
	}

}