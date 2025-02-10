#include "pch.h"
#include "FileSystem.h"
#include "core/io/Logging.h"


namespace trace {

	FileSystem::FileSystem()
	{
	}
	FileSystem::~FileSystem()
	{
	}

	bool FileSystem::Init()
	{
		return false;
	}

	void FileSystem::Shutdown()
	{

	}

	bool FileSystem::open_file(const std::string& filename, FileMode mode, FileHandle& out_handle)
	{
		
		std::ios_base::openmode file_mode = 0;

		if (mode & FileMode::READ)
		{
			file_mode |= std::ios::in;
		}
		if (mode & FileMode::WRITE)
		{
			file_mode |= std::ios::out;
		}
		if (mode & FileMode::BINARY)
		{
			file_mode |= std::ios::binary;
		}
		if (mode & FileMode::APPEND)
		{
			file_mode |= std::ios::app;
		}
		if (mode & FileMode::TRUNC)
		{
			file_mode |= std::ios::trunc;
		}

		std::fstream* file = new std::fstream();

		file->open(filename, file_mode);

		if (file->is_open())
		{
			out_handle.m_isVaild = true;
			out_handle.m_handle = file;
			return true;
		}
	
		
		out_handle.m_handle = nullptr;
		out_handle.m_isVaild = false;

		return false;
	}
	bool FileSystem::open_file(const char* file_name, FileMode mode, FileHandle& out_handle)
	{
		std::ios_base::openmode file_mode = 0;

		if (mode & FileMode::READ)
		{
			file_mode |= std::ios::in;
		}
		if (mode & FileMode::WRITE)
		{
			file_mode |= std::ios::out;
		}
		if (mode & FileMode::BINARY)
		{
			file_mode |= std::ios::binary;
		}
		if (mode & FileMode::APPEND)
		{
			file_mode |= std::ios::app;
		}
		if (mode & FileMode::TRUNC)
		{
			file_mode |= std::ios::trunc;
		}

		std::fstream* file = new std::fstream();

		file->open(file_name, file_mode);

		if (file->is_open())
		{
			out_handle.m_isVaild = true;
			out_handle.m_handle = file;
			return true;
		}


		out_handle.m_handle = nullptr;
		out_handle.m_isVaild = false;

		return false;
	}
	void FileSystem::close_file(FileHandle& file)
	{
		std::fstream* _file = ((std::fstream*)file.m_handle);

		if (file.m_isVaild)
		{
			_file->close();
			file.m_isVaild = false;
			delete _file;
		}
	}

	void FileSystem::read(void* destination, size_t sizeBytes, FileHandle& file)
	{

		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			_file->read((char*)destination, sizeBytes);
			return;
		}
		TRC_ERROR("File has to be vaild to be able to read from {}", file.m_handle);

	}
	void FileSystem::write(void* data, size_t sizeBytes, FileHandle& file)
	{
		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			_file->write((const char*)data, sizeBytes);
			return;
		}
		TRC_ERROR("File has to be vaild to be able to write to {}", file.m_handle);

	}
	void FileSystem::readline(FileHandle& file,std::string& out_line)
	{
		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			std::getline(*_file, out_line);
			return;
		}
		TRC_ERROR("File has to be vaild to be able to read from {}", file.m_handle);
	}
	void FileSystem::writestring(FileHandle& file, const std::string& line)
	{
		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			*_file << line;
			return;
		}
		TRC_ERROR("File has to be vaild to be able to write to {}", file.m_handle);
	}
	void FileSystem::read_all_bytes(FileHandle& file, void* data, uint32_t& read_bytes)
	{
		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			size_t begin = _file->tellg();
			_file->seekg(0, std::ios::end);
			size_t end = _file->tellg();
			read_bytes = static_cast<uint32_t>(end - begin);
			_file->seekg(0, std::ios::beg);
			if (!data) return;
			read(data, read_bytes, file);
			return;
		}
		TRC_ERROR("File has to be vaild to be able to read from {}", file.m_handle);
	}
	void FileSystem::read_all_lines(FileHandle& file, std::string& out)
	{
		if (file.m_isVaild)
		{
			std::fstream* _file = (std::fstream*)file.m_handle;
			_file->seekg(0);

			std::stringstream result;
			std::string str;
			while (std::getline(*_file, str))
			{
				result << str << "\n";
			}
			out = result.str();
			return;
		}
		TRC_ERROR("File has to be vaild to be able to read from {}", file.m_handle);
	}

	void FileSystem::set_seek_position(FileHandle& file, size_t pos)
	{

		std::fstream* _file = ((std::fstream*)file.m_handle);

		if (file.m_isVaild)
		{
			_file->seekg(pos, std::ios::beg);
			_file->seekp(pos, std::ios::beg);
		}

	}

}