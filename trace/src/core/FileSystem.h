#pragma once

#include <core/Enums.h>
#include <core/Core.h>
#include <EASTL/string.h>

namespace trace {

	enum FileMode
	{
		READ = 0x01,
		WRITE = 0x02,
		BINARY = 0x04,
		APPEND = 0x08,
		TRUNC = 0x10
	};

	struct FileHandle
	{
		void* m_handle = nullptr;
		bool m_isVaild = false;
	};

	class TRACE_API FileSystem
	{

	public:
		FileSystem();
		~FileSystem();

		static bool open_file(const std::string& filename, FileMode mode, FileHandle& out_handle);
		static bool open_file(const char* file_name, FileMode mode, FileHandle& out_handle);

		static void close_file(FileHandle& file);

		static void read(void* destination, size_t sizeBytes, FileHandle& file);
		static void write(void* data, size_t sizeBytes, FileHandle& file);

		static void readline(FileHandle& file, std::string& out_line);
		static void writeline(FileHandle& file, const std::string& line);

		static void read_all_bytes(FileHandle& file, void* data, uint32_t& read_bytes);
		static void read_all_lines(FileHandle& file, std::string& out);

	private:
	protected:

	};

}